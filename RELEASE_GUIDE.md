# Release Guide

This guide explains how to create and publish releases for BigBrother.

## Quick Release Process

### 1. Build the Project

From a Developer Command Prompt:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 2. Package for Release

Run the packaging script:
```bash
cd scripts
package.bat
```

This creates `bigbro-vX.X.X.zip` with all necessary files.

### 3. Update Version

Before packaging, update the version in `VERSION` file.

### 4. Create GitHub Release

1. Go to GitHub → Releases → Draft a new release
2. Create tag (e.g., `v1.0.0`)
3. Copy release notes from `scripts/RELEASE_NOTES_TEMPLATE.md`
4. Upload `bigbro-vX.X.X.zip`
5. Publish!

## Testing

Before publishing:
1. Extract ZIP to fresh folder
2. Run `install.ps1`
3. Test viewer functionality
4. Run `uninstall.ps1`

## Version Numbering

Semantic Versioning: `MAJOR.MINOR.PATCH`
- 1.0.0 - Initial release
- 1.1.0 - New features
- 1.1.1 - Bug fixes
