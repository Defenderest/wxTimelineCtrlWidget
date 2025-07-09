# Build Status Badges

With the new separate workflow structure, you can now get granular build status information for each platform and configuration.

## Individual Platform Badges

Add these badges to your README.md to show the status of each platform separately:

### Windows Build Status
```markdown
[![Windows Build](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-windows.yml/badge.svg)](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-windows.yml)
```

### Linux Build Status
```markdown
[![Linux Build](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-linux.yml/badge.svg)](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-linux.yml)
```

### macOS Build Status
```markdown
[![macOS Build](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-macos.yml/badge.svg)](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-macos.yml)
```

## All Platforms in One Table

```markdown
| Platform | Status |
|----------|--------|
| Windows  | [![Windows Build](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-windows.yml/badge.svg)](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-windows.yml) |
| Linux    | [![Linux Build](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-linux.yml/badge.svg)](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-linux.yml) |
| macOS    | [![macOS Build](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-macos.yml/badge.svg)](https://github.com/YOUR_USERNAME/wxTimelineCtrl_github/actions/workflows/build-macos.yml) |
```

## Benefits of the New Structure

### 1. **Granular Status Visibility**
- If Windows Debug fails but Windows Release passes, you'll see exactly which configuration is having issues
- Each OS has its own independent badge - a Linux failure won't make the Windows badge show as failed

### 2. **Separate Job Results**
- Windows builds are split into `Windows Debug` and `Windows Release` jobs
- Linux and macOS each run both Debug and Release configurations in a matrix
- Artifact uploads are specific to each platform and configuration

### 3. **Better Caching**
- Windows jobs share the wxWidgets cache between Debug and Release builds
- Each platform caches independently, improving build times

### 4. **Individual Job Control**
- You can re-run just the failed platform without re-running successful ones
- Easier to debug platform-specific issues

## Replace YOUR_USERNAME

Don't forget to replace `YOUR_USERNAME` in the badge URLs with your actual GitHub username!

## Workflow File Locations

The workflows are located at:
- `.github/workflows/build-windows.yml` - Windows builds (Debug and Release jobs)
- `.github/workflows/build-linux.yml` - Linux builds (Debug and Release matrix)
- `.github/workflows/build-macos.yml` - macOS builds (Debug and Release matrix)
- `.github/workflows/build-legacy.yml` - Old combined workflow (can be deleted)
