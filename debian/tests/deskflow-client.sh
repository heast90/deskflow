#!/usr/bin/bash

TEST_DIR=$(dirname $(realpath $0))
# e.g. 1.22.0+dfsg-3 => 1.22.0
CHANGELOG_VERSION=$(dpkg-parsechangelog -l$TEST_DIR/../changelog | grep Version | cut -d' ' -f2 | cut -d'+' -f1)
# e.g v1.22.0.0, => v1.22.0.0
PACKAGE_VERSION=$(deskflow-client --version | head -n 1 | cut -d' ' -f2 | sed -e 's/,//')

echo "Testing $0"

if [ "$PACKAGE_VERSION" = "v${CHANGELOG_VERSION}.0" ]; then
    echo "Pass"
else
    echo "deskflow-client: package version and changelog version must be matched"
    echo "Expected: $CHANGELOG_VERSION Actual: $PACKAGE_VERSION"
    exit 1
fi
