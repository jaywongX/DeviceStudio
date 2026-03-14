#!/bin/bash

echo "========================================"
echo "DeviceStudio Build Script for Linux"
echo "========================================"
echo ""

# 设置 Qt 路径 (如需要)
# export Qt6_DIR=/path/to/Qt/6.6.0/gcc_64/lib/cmake/Qt6

# 创建构建目录
echo "[1/4] Creating build directory..."
mkdir -p build
cd build

# 配置
echo ""
echo "[2/4] Configuring CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: CMake configuration failed!"
    exit 1
fi

# 构建 (使用所有核心)
echo ""
echo "[3/4] Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed!"
    exit 1
fi

# 运行测试
echo ""
echo "[4/4] Running tests..."
ctest --output-on-failure

echo ""
echo "========================================"
echo "Build complete!"
echo "Executable: build/bin/DeviceStudio"
echo "========================================"
