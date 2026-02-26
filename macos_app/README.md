# macOS App Bundle for TRS-80 Level II BASIC

This directory contains the files needed to build a macOS application bundle for the BASIC interpreter.

## Building the App

To build the app bundle:

```bash
make app
```

This will create `TRS-80 Level II BASIC.app` in the project root directory.

## Installing to Applications

To install the app to `/Applications`:

```bash
make install-app
```

After installation, you can:
- Launch it from Applications folder
- Launch it from Spotlight (Cmd+Space, type "TRS-80")
- Launch it from Launchpad

## App Structure

```
TRS-80 Level II BASIC.app/
  Contents/
    MacOS/
      basic-trs80          # The BASIC interpreter binary
      launcher.sh          # Launcher script
    Resources/             # (empty for now, icon can go here)
    Info.plist            # App metadata
```

## Custom Icon

To add a custom icon:

1. Create an icon set using `iconutil`:
   ```bash
   mkdir AppIcon.iconset
   # Add icon files: icon_16x16.png, icon_32x32.png, etc.
   iconutil -c icns AppIcon.iconset
   ```

2. Copy the `.icns` file to `Contents/Resources/AppIcon.icns`

3. The Info.plist already references "AppIcon" as the icon file.

## How It Works

When you launch the app:
1. The launcher script (`launcher.sh`) is executed
2. It finds the BASIC interpreter binary in `Contents/MacOS/`
3. It opens a new Terminal window with:
   - 64x16 character size (TRS-80 style)
   - Black background, white text
   - Courier font
   - Runs the BASIC interpreter in interactive mode

## Troubleshooting

If the app doesn't launch:
- Check that the binary exists: `TRS-80 Level II BASIC.app/Contents/MacOS/basic-trs80`
- Check that the launcher is executable: `chmod +x TRS-80\ Level\ II\ BASIC.app/Contents/MacOS/launcher.sh`
- Rebuild the app: `make clean && make app`
