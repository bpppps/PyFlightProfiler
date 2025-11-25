PYTHON_VERSION=$1
PYTHON_TGZ="Python-${PYTHON_VERSION}.tgz"
PYTHON_URL="https://www.python.org/ftp/python/${PYTHON_VERSION}/${PYTHON_TGZ}"
PYTHON_INSTALL_DIR="/usr/local/python-$PYTHON_VERSION"

wget $PYTHON_URL
tar -xzf "${PYTHON_TGZ}"
cd "Python-${PYTHON_VERSION}"
./configure --enable-optimizations --prefix="${PYTHON_INSTALL_DIR}"
make
make install
cd ..
rm -rf "Python-${PYTHON_VERSION}" "${PYTHON_TGZ}"
echo "install python $PYTHON_VERSION successfully to $PYTHON_INSTALL_DIR"
