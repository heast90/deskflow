/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "platform/WlClipboard.h"

#include "base/Log.h"

#include <chrono>
#include <common/Settings.h>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <QDateTime>
#include <QStandardPaths>

namespace {

inline static const auto s_copyApp = QStringLiteral("wl-copy");
inline static const auto s_pasteApp = QStringLiteral("wl-paste");

// wl-clipboard args
inline static const auto s_listTypes = QStringLiteral("--list-types");
inline static const auto s_isPrimary = QStringLiteral("--primary");
inline static const auto s_noNewLine = QStringLiteral("-n");
inline static const auto s_foreground = QStringLiteral("--foreground");
inline static const auto s_readType = QStringLiteral("-t%1");

// MIME types for different clipboard formats
inline static const auto s_mimeTypeText = QStringLiteral("text/plain");
inline static const auto s_mimeTypeHtml = QStringLiteral("text/html");
inline static const auto s_mimeTypeBmp = QStringLiteral("image/bmp");

// Additional HTML MIME type variants
const char *const s_mimeTypeHtmlUtf8 = "text/html;charset=UTF-8";
const char *const s_mimeTypeHtmlWindows = "HTML Format";

// Command timeout (milliseconds)
const int kCacheValidityMs = 100;
const int kMonitorIntervalMs = 1000;
const int kMaxConsecutiveErrors = 5;
} // namespace

// RAII helper: install SIGCHLD handler once per process
namespace {
struct SigChldGuard {
  SigChldGuard() {
    struct sigaction sa;
    sa.sa_handler = +[](int) {
      // Reap ALL finished children. This is critical: QProcess on this
      // platform does NOT automatically reap children. Without this handler,
      // any wl-copy child that exits becomes a zombie until someone
      // explicitly calls waitpid().
      while (::waitpid(-1, nullptr, WNOHANG) > 0) {}
    };
    ::sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    ::sigaction(SIGCHLD, &sa, nullptr);
  }
};
static SigChldGuard s_sigchldGuard;
}

WlClipboard::WlClipboard(ClipboardID id) : m_id(id), m_useClipboard(id == kClipboardClipboard)
{
  // Initialize cached data
  for (int i = 0; i < static_cast<int>(Format::TotalFormats); ++i) {
    m_cachedAvailable[i] = false;
  }

  // Periodic timer to clean up finished PIDs from m_runningWlCopies.
  // The SIGCHLD handler reaps children at the kernel level, but we still
  // need to update our tracking list to detect when processes have exited.
  m_reaperTimer = new QTimer(this);
  m_reaperTimer->setInterval(1000);
  connect(m_reaperTimer, &QTimer::timeout, this, &WlClipboard::reapDeadWlCopies);
  m_reaperTimer->start();
}

WlClipboard::~WlClipboard()
{
  if (m_reaperTimer) {
    m_reaperTimer->stop();
  }
  stopMonitoring();
  for (auto &entry : m_runningWlCopies) {
    pid_t pid = entry.first;
    if (pid > 0) {
      ::kill(pid, SIGTERM);
      // Wait a bit for the process to exit. The SIGCHLD handler will reap it.
      for (int i = 0; i < 50; ++i) {
        if (::kill(pid, 0) < 0 && errno == ESRCH) break;
        ::usleep(10000);  // 10ms
      }
      if (::kill(pid, 0) == 0) {
        ::kill(pid, SIGKILL);
        ::usleep(50000);
      }
    }
  }
  m_runningWlCopies.clear();
}

ClipboardID WlClipboard::getID() const
{
  return m_id;
}

bool WlClipboard::isAvailable()
{
  LOG_DEBUG("[CLIP-CU-WC-001] WlClipboard.cpp:72 isAvailable() -- checking availability");
  return !QStandardPaths::findExecutable(s_copyApp).isEmpty() && !QStandardPaths::findExecutable(s_pasteApp).isEmpty();
}

bool WlClipboard::isEnabled()
{
  return Settings::value(Settings::Core::UseWlClipboard).toBool();
}

void WlClipboard::startMonitoring()
{
  LOG_DEBUG("[CLIP-CU-WC-002] WlClipboard.cpp:82 startMonitoring() -- starting clipboard monitoring");
  if (m_monitoring) {
    return;
  }
  m_stopMonitoring = false;
  m_monitoring = true;
  m_monitorThread = std::make_unique<std::thread>(&WlClipboard::monitorClipboard, this);
}

void WlClipboard::stopMonitoring()
{
  LOG_DEBUG("[CLIP-CU-WC-003] WlClipboard.cpp:92 stopMonitoring() -- stopping clipboard monitoring");
  if (!m_monitoring) {
    return;
  }

  m_stopMonitoring = true;
  m_monitoring = false;

  if (m_monitorThread && m_monitorThread->joinable()) {
    m_monitorThread->join();
  }
  m_monitorThread.reset();
}

bool WlClipboard::hasChanged() const
{
  LOG_DEBUG("[CLIP-CU-WC-004] WlClipboard.cpp:107 hasChanged() -- checking if clipboard changed");
  return m_hasChanged.load();
}

bool WlClipboard::empty()
{
  LOG_DEBUG("[CLIP-CU-WC-005] WlClipboard.cpp:112 empty() -- emptying clipboard, m_open=%d", m_open);
  if (!m_open) {
    return false;
  }

  // Do NOT start a separate wl-copy process here. Starting a wl-copy for the
  // empty string races with the subsequent add() wl-copy (which writes the
  // actual data). On Wayland, the last wl-copy to complete its protocol
  // handshake wins. If the empty() wl-copy completes after add(), the
  // clipboard gets cleared, overwriting our data.
  //
  // Instead, just claim ownership locally. The add() call will start a
  // single wl-copy that both asserts ownership AND writes the actual data.
  std::scoped_lock<std::mutex> lock(m_cacheMutex);
  updateOwnership(true);
  invalidateCache();

  return true;
}

void WlClipboard::add(Format format, const std::string &data)
{
  LOG_DEBUG("[CLIP-CU-WC-006] WlClipboard.cpp:140 add() -- adding data, format=%d, size=%zu, m_open=%d", format, data.size(), m_open);
  if (!m_open) {
    return;
  }

  if (format == Format::HTML) {
    return;
  }

  auto mimeType = formatToMimeType(format);
  if (mimeType.isEmpty()) {
    LOG_WARN("unsupported clipboard format: %d", format);
    return;
  }

  // Build argv for wl-copy --foreground
  // Use raw fork/exec instead of QProcess because QProcess on this platform
  // does NOT automatically reap children in the event loop. With QProcess,
  // a wl-copy that exits naturally becomes a permanent zombie. We use raw
  // fork/exec with a SIGCHLD handler (installed at static init time) that
  // reaps ALL finished children immediately.
  QByteArray appPath = s_copyApp.toUtf8();
  QByteArray flagNoNewline = s_noNewLine.toUtf8();
  QByteArray flagForeground = s_foreground.toUtf8();
  QByteArray flagReadType = s_readType.arg(mimeType).toUtf8();
  QByteArray dataBytes = QString::fromStdString(data).toUtf8();
  QByteArray flagPrimary = s_isPrimary.toUtf8();

  // Max 6 args: app, -n, --foreground, -t<type>, <data>, --primary (optional), null
  const char *argv[7];
  int ai = 0;
  argv[ai++] = appPath.constData();
  argv[ai++] = flagNoNewline.constData();
  argv[ai++] = flagForeground.constData();
  argv[ai++] = flagReadType.constData();
  if (!m_useClipboard)
    argv[ai++] = flagPrimary.constData();
  argv[ai++] = dataBytes.constData();
  argv[ai] = nullptr;

  pid_t pid = ::fork();
  if (pid < 0) {
    LOG_WARN("[CLIP-CU-WC-014] add() fork failed");
    return;
  }

  if (pid == 0) {
    // Child: exec wl-copy
    // NOTE: Do NOT use any Qt API (QByteArray, QString, etc.) after fork()!
    // Qt is NOT fork-safe. Use only C library functions.
    ::setsid();  // new session so process doesn't get killed with parent
    ::execvp(argv[0], const_cast<char *const *>(argv));
    ::_exit(EXIT_FAILURE);
  }

  // Parent: track PID
  m_runningWlCopies.push_back({pid, false});

  LOG_DEBUG("[CLIP-CU-WC-009] WlClipboard.cpp:165 add() -- wl-copy started, pid=%d, foreground=1", pid);

  std::scoped_lock<std::mutex> lock(m_cacheMutex);
  updateOwnership(true);
  invalidateCache();
}

void WlClipboard::reapDeadWlCopies()
{
  // The SIGCHLD handler (installed at static init) already calls
  // waitpid(-1, WNOHANG) whenever a child exits. But we still need to
  // track which PIDs have finished and clean up our records.
  //
  // For each tracked PID, check if it's still alive. If kill(pid, 0)
  // returns ESRCH, the process is gone (already reaped by SIGCHLD handler).
  for (auto it = m_runningWlCopies.begin(); it != m_runningWlCopies.end(); ) {
    pid_t pid = it->first;
    if (pid > 0 && ::kill(pid, 0) < 0 && errno == ESRCH) {
      LOG_DEBUG("[CLIP-CU-WC-013] wl-copy finished (reaper), pid=%d", pid);
      it = m_runningWlCopies.erase(it);
    } else {
      ++it;
    }
  }
}

bool WlClipboard::open(Time time) const
{
  LOG_DEBUG("[CLIP-CU-WC-007] WlClipboard.cpp:176 open() -- opening clipboard, m_open=%d, time=%u", m_open, time);
  if (m_open) {
    LOG_DEBUG("failed to open clipboard: already opened");
    return false;
  }

  m_open = true;
  m_time = time;

  return true;
}

void WlClipboard::close() const
{
  LOG_DEBUG("[CLIP-CU-WC-008] WlClipboard.cpp:189 close() -- closing clipboard, m_open=%d", m_open);
  if (!m_open) {
    return;
  }

  LOG_DEBUG("close clipboard");

  m_open = false;
  const_cast<WlClipboard *>(this)->invalidateCache();
}

IClipboard::Time WlClipboard::getTime() const
{
  return m_time;
}

bool WlClipboard::has(Format format) const
{
  if (!m_open) {
    return false;
  }

  std::scoped_lock<std::mutex> lock(m_cacheMutex);

  // Check cache validity
  Time currentTime = getCurrentTime();
  if (m_cached && (currentTime - m_cachedTime) < kCacheValidityMs) {
    return m_cachedAvailable[static_cast<int>(format)];
  }

  if (const auto availableTypes = getAvailableMimeTypes(); availableTypes.isEmpty()) {
    // No types available - mark all formats as unavailable
    for (int i = 0; i < static_cast<int>(Format::TotalFormats); ++i) {
      m_cachedAvailable[i] = false;
      m_cachedData[i].clear();
    }
  } else {
    using enum IClipboard::Format;
    // Check each format against available types
    for (int i = 0; i < static_cast<int>(TotalFormats); ++i) {
      auto currentFormat = static_cast<Format>(i);
      const auto mimeType = formatToMimeType(currentFormat);

      m_cachedAvailable[i] = false;
      if (!mimeType.isEmpty()) {
        for (const auto &available : availableTypes) {
          if (available == mimeType || (currentFormat == Text && available == QStringLiteral("text/plain")) ||
              (currentFormat == HTML && available.startsWith(QStringLiteral("text/html")))) {
            m_cachedAvailable[i] = true;
            break;
          }
        }
      }
    }
  }

  m_cached = true;
  m_cachedTime = currentTime;

  return m_cachedAvailable[static_cast<int>(format)];
}

std::string WlClipboard::get(Format format) const
{
  LOG_DEBUG("[CLIP-CU-WC-010] WlClipboard.cpp:254 get() -- reading clipboard data, format=%d, m_open=%d, m_cached=%d, m_cachedAvailable[fmt]=%d, cachedSize=%zu", format, m_open, m_cached, m_cachedAvailable[static_cast<int>(format)], m_cachedData[static_cast<int>(format)].size());
  if (!m_open) {
    return std::string();
  }

  std::scoped_lock<std::mutex> lock(m_cacheMutex);

  // Return cached data if available and valid
  if (m_cached && m_cachedAvailable[static_cast<int>(format)] && !m_cachedData[static_cast<int>(format)].empty()) {
    LOG_DEBUG("[CLIP-CU-WC-011] WlClipboard.cpp:262 get() -- returning cached data, size=%zu", m_cachedData[static_cast<int>(format)].size());
    return m_cachedData[static_cast<int>(format)];
  }

  auto mimeType = formatToMimeType(format);
  if (mimeType.isEmpty()) {
    return std::string();
  }

  // Use raw fork/exec with pipe (same approach as getAvailableMimeTypes)
  // to avoid QProcess, which doesn't properly reap children on this platform.
  int pipefd[2];
  if (::pipe2(pipefd, O_CLOEXEC) < 0) {
    LOG_WARN("[CLIP-CU-WC-016] get() pipe failed");
    return std::string();
  }

  // Build argv BEFORE fork (Qt is NOT fork-safe - use no Qt API after fork)
  QByteArray appPath = s_pasteApp.toUtf8();
  QByteArray flagNoNewline = s_noNewLine.toUtf8();
  QByteArray flagReadType = s_readType.arg(mimeType).toUtf8();
  QByteArray flagPrimary = s_isPrimary.toUtf8();

  const char *argv[5];
  int ai = 0;
  argv[ai++] = appPath.constData();
  argv[ai++] = flagNoNewline.constData();
  argv[ai++] = flagReadType.constData();
  if (!m_useClipboard)
    argv[ai++] = flagPrimary.constData();
  argv[ai] = nullptr;

  pid_t pid = ::fork();
  if (pid < 0) {
    LOG_WARN("[CLIP-CU-WC-016] get() fork failed");
    ::close(pipefd[0]);
    ::close(pipefd[1]);
    return std::string();
  }

  if (pid == 0) {
    // Child: exec wl-paste -n -t<type>
    // NOTE: No Qt API after fork! Qt is NOT fork-safe.
    ::close(pipefd[0]);
    ::dup2(pipefd[1], STDOUT_FILENO);
    ::close(pipefd[1]);

    ::execvp(argv[0], const_cast<char *const *>(argv));
    // If execvp returns, it failed
    ::_exit(EXIT_FAILURE);
  }

  LOG_DEBUG("[CLIP-CU-WC-016] get() forked, pid=%d", pid);
  // Parent: close write end, poll for data with timeout
  ::close(pipefd[1]);

  struct pollfd pfd;
  pfd.fd = pipefd[0];
  pfd.events = POLLIN | POLLHUP;

  const int kGetTimeoutMs = 3000;
  bool timedOut = (::poll(&pfd, 1, kGetTimeoutMs) == 0);

  std::string data;
  if (!timedOut) {
    // Only read pipe data if poll succeeded (not on timeout), otherwise
    // ::read() blocks forever if child is still alive with pipe open.
    char buf[4096];
    ssize_t n;
    while ((n = ::read(pipefd[0], buf, sizeof(buf))) > 0) {
      data.append(buf, n);
    }
  }
  ::close(pipefd[0]);

  // Reap the child (SIGCHLD handler may already have done this, but
  // waitpid with specific PID is safe to call either way).
  if (timedOut) {
    LOG_WARN("[CLIP-CU-WC-016] get() timed out after %dms, killing wl-paste", kGetTimeoutMs);
    ::kill(pid, SIGKILL);
  }
  ::waitpid(pid, nullptr, 0);

  if (timedOut) {
    return std::string();
  }

  LOG_DEBUG("[CLIP-CU-WC-012] WlClipboard.cpp:290 get() -- wl-paste returned %zu bytes: '%s'", data.size(), data.c_str());

  // Update cache
  m_cachedData[static_cast<int>(format)] = data;
  m_cachedAvailable[static_cast<int>(format)] = !data.empty();
  m_cached = true;
  m_cachedTime = getCurrentTime();

  return data;
}

QString WlClipboard::formatToMimeType(Format format) const
{
  switch (format) {
    using enum IClipboard::Format;
  case Text:
    return s_mimeTypeText;
  case HTML:
    return s_mimeTypeHtml;
  case Bitmap:
    return s_mimeTypeBmp;
  default:
    return {};
  }
}

IClipboard::Format WlClipboard::mimeTypeToFormat(const QString &mimeType) const
{
  using enum IClipboard::Format;
  if (mimeType == s_mimeTypeText || mimeType == QStringLiteral("text/plain") || mimeType == QStringLiteral("text/plain;charset=utf-8")) {
    return Text;
  }
  if (mimeType == s_mimeTypeHtml || mimeType == s_mimeTypeHtmlUtf8 || mimeType == s_mimeTypeHtmlWindows ||
      mimeType.contains("text/html")) {
    return HTML;
  }
  if (mimeType == s_mimeTypeBmp) {
    return Bitmap;
  }
  return Text; // Default fallback
}

QStringList WlClipboard::getAvailableMimeTypes(int timeoutMs) const
{
  // IMPORTANT: This function MUST NOT use QProcess for the wl-paste child.
  // QProcess internally calls waitpid(-1, WNOHANG) which reaps ANY child of
  // the process, not just its own. If this function (called from the
  // monitoring std::thread) uses QProcess, its waitpid(-1) can race with
  // the main thread's QProcess and steal the exit status of wl-copy children
  // belonging to the main thread. This leaves wl-copy as an un-reaped zombie.
  //
  // Instead, we use raw fork() + execvp() + waitpid(pid, ...) with a
  // specific PID. The pipe-based poll() provides the timeout mechanism.

  int pipefd[2];
  if (::pipe2(pipefd, O_CLOEXEC) < 0) {
    LOG_WARN("[CLIP-CU-WC-015] getAvailableMimeTypes() pipe failed");
    return {};
  }

  // Build argv BEFORE fork (Qt is NOT fork-safe)
  QByteArray appPath = s_pasteApp.toUtf8();
  QByteArray flagPrimary = s_isPrimary.toUtf8();
  QByteArray flagListTypes = s_listTypes.toUtf8();

  const char *argv[4];
  int ai = 0;
  argv[ai++] = appPath.constData();
  if (!m_useClipboard)
    argv[ai++] = flagPrimary.constData();
  argv[ai++] = flagListTypes.constData();
  argv[ai] = nullptr;

  pid_t pid = ::fork();
  if (pid < 0) {
    LOG_WARN("[CLIP-CU-WC-015] getAvailableMimeTypes() fork failed");
    ::close(pipefd[0]);
    ::close(pipefd[1]);
    return {};
  }

  if (pid == 0) {
    // Child: exec wl-paste --list-types [--primary]
    // NOTE: No Qt API after fork! Qt is NOT fork-safe.
    ::close(pipefd[0]);
    ::dup2(pipefd[1], STDOUT_FILENO);
    ::close(pipefd[1]);

    ::execvp(argv[0], const_cast<char *const *>(argv));
    // If execvp returns, it failed
    ::_exit(EXIT_FAILURE);
  }

  // Parent: close write end
  ::close(pipefd[1]);

  // Poll for data with timeout
  struct pollfd pfd;
  pfd.fd = pipefd[0];
  pfd.events = POLLIN | POLLHUP;
  bool timedOut = (::poll(&pfd, 1, timeoutMs) == 0);

  std::string output;
  if (!timedOut) {
    // Read any data from pipe (child may have written output).
    // Only do this if poll reported data, NOT after timeout - otherwise
    // ::read() blocks forever if child is still alive (no data, pipe open).
    char buf[4096];
    ssize_t n;
    while ((n = ::read(pipefd[0], buf, sizeof(buf))) > 0) {
      output.append(buf, n);
    }
  }
  ::close(pipefd[0]);

  // Reap child with specific PID (NOT waitpid(-1)) to avoid stealing
  // children from the main thread's QProcess instances.
  int status = 0;
  if (timedOut) {
    LOG_DEBUG("[CLIP-CU-WC-015] getAvailableMimeTypes() timed out after %dms, killing wl-paste", timeoutMs);
    ::kill(pid, SIGKILL);
  }
  ::waitpid(pid, &status, 0);  // blocking wait, child exits quickly

  if (timedOut) {
    return {};
  }

  return QString::fromStdString(output).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
}

void WlClipboard::monitorClipboard()
{
  QStringList lastTypes;
  int consecutiveErrors = 0;

  while (!m_stopMonitoring) {
    std::this_thread::sleep_for(std::chrono::milliseconds(kMonitorIntervalMs));
    try {
      // Check if clipboard content has changed by comparing available types
      const auto currentTypes = getAvailableMimeTypes();

      // Reset error counter on successful operation
      consecutiveErrors = 0;

      if (currentTypes != lastTypes) {
        m_hasChanged = true;
        lastTypes = currentTypes;

        // Clear cache when clipboard changes
        std::scoped_lock<std::mutex> lock(m_cacheMutex);
        invalidateCache();
        updateOwnership(false);
      }
    } catch (const std::exception &e) {
      LOG_WARN("clipboard monitoring error: %s", e.what());
      if (++consecutiveErrors >= kMaxConsecutiveErrors) {
        LOG_ERR("too many consecutive errors in clipboard monitoring, stopping");
        break;
      }
    } catch (...) {
      LOG_WARN("clipboard monitoring unknown error");
      if (++consecutiveErrors >= kMaxConsecutiveErrors) {
        LOG_ERR("too many consecutive errors in clipboard monitoring, stopping");
        break;
      }
    }
  }
}

IClipboard::Time WlClipboard::getCurrentTime() const
{
  return static_cast<Time>(QDateTime::currentMSecsSinceEpoch());
}

bool WlClipboard::isOwned() const
{
  return m_owned;
}

void WlClipboard::resetChanged()
{
  m_hasChanged = false;

  // Clear cache when resetting change flag to force fresh data retrieval
  std::scoped_lock<std::mutex> lock(m_cacheMutex);
  invalidateCache();
}

void WlClipboard::updateOwnership(bool owned)
{
  m_owned = owned;
}

void WlClipboard::invalidateCache()
{
  m_cached = false;
  m_cachedTime = 0;
  for (int i = 0; i < static_cast<int>(Format::TotalFormats); ++i) {
    m_cachedData[i].clear();
    m_cachedAvailable[i] = false;
  }
}
