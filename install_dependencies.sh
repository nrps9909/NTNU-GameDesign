#!/bin/bash

echo "Updating package lists..."
sudo apt update

echo "Installing core build tools..."
sudo apt install -y build-essential

echo "Installing OpenGL development libraries..."
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev

echo "Installing Wayland development libraries..."
sudo apt install -y wayland-protocols libwayland-dev

echo "Installing pkg-config..."
sudo apt install -y pkg-config

echo "Installing X11 development libraries..."
sudo apt install -y \
    libxrandr-dev \
    libxkbcommon-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev

# Optional:
# echo "Installing dos2unix..."
# sudo apt install -y dos2unix

echo "All specified dependencies have been processed."
echo "You might still encounter further dependencies based on your project's specific needs."