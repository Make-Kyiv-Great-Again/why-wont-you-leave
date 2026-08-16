#!/bin/bash
set -e

echo "=== Building Game for macOS ==="
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

echo "=== Packaging macOS Distribution ==="
rm -rf dist/WhyWontYouLeave dist/WhyWontYouLeave-macOS.zip
mkdir -p dist/WhyWontYouLeave

cp build/RaylibGame dist/WhyWontYouLeave/
cp -r assets dist/WhyWontYouLeave/

# Create double-clickable .command launcher
cat << 'EOF' > dist/WhyWontYouLeave/Play_Game.command
#!/bin/bash
cd "$(dirname "$0")"
chmod +x ./RaylibGame
./RaylibGame
EOF

chmod +x dist/WhyWontYouLeave/Play_Game.command
chmod +x dist/WhyWontYouLeave/RaylibGame

cd dist
zip -r WhyWontYouLeave-macOS.zip WhyWontYouLeave
cd ..

echo "=== Packaging Complete! ==="
echo "Output: dist/WhyWontYouLeave-macOS.zip"
