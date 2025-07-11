/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "wxTimelineCtrlWidget", "index.html", [
    [ "wxTimelineCtrl API Reference", "index.html", "index" ],
    [ "Build Status Badges", "md_BUILD__BADGES.html", [
      [ "Individual Platform Badges", "md_BUILD__BADGES.html#autotoc_md1", [
        [ "Windows Build Status", "md_BUILD__BADGES.html#autotoc_md2", null ],
        [ "Linux Build Status", "md_BUILD__BADGES.html#autotoc_md3", null ],
        [ "macOS Build Status", "md_BUILD__BADGES.html#autotoc_md4", null ]
      ] ],
      [ "All Platforms in One Table", "md_BUILD__BADGES.html#autotoc_md5", null ],
      [ "Benefits of the New Structure", "md_BUILD__BADGES.html#autotoc_md6", [
        [ "1. <strong>Granular Status Visibility</strong>", "md_BUILD__BADGES.html#autotoc_md7", null ],
        [ "2. <strong>Separate Job Results</strong>", "md_BUILD__BADGES.html#autotoc_md8", null ],
        [ "3. <strong>Better Caching</strong>", "md_BUILD__BADGES.html#autotoc_md9", null ],
        [ "4. <strong>Individual Job Control</strong>", "md_BUILD__BADGES.html#autotoc_md10", null ]
      ] ],
      [ "Replace YOUR_USERNAME", "md_BUILD__BADGES.html#autotoc_md11", null ],
      [ "Workflow File Locations", "md_BUILD__BADGES.html#autotoc_md12", null ]
    ] ],
    [ "wxTimelineCtrl", "md_README.html", [
      [ "📋 Table of Contents", "md_README.html#autotoc_md18", null ],
      [ "✨ Features", "md_README.html#autotoc_md19", null ],
      [ "🚀 Quick Start", "md_README.html#autotoc_md20", [
        [ "Prerequisites", "md_README.html#autotoc_md21", null ],
        [ "Build and Run", "md_README.html#autotoc_md22", null ]
      ] ],
      [ "📁 Project Structure", "md_README.html#autotoc_md23", null ],
      [ "🔨 Building the Project", "md_README.html#autotoc_md24", [
        [ "Setting up wxWidgets", "md_README.html#autotoc_md25", null ],
        [ "Build Options", "md_README.html#autotoc_md26", [
          [ "Option 1: Using Build Scripts (Recommended)", "md_README.html#autotoc_md27", null ],
          [ "Option 2: Manual CMake Build", "md_README.html#autotoc_md28", null ]
        ] ],
        [ "Build Output", "md_README.html#autotoc_md29", null ]
      ] ],
      [ "🎮 Using the Sample Application", "md_README.html#autotoc_md30", [
        [ "Interface Overview", "md_README.html#autotoc_md31", null ],
        [ "Interaction Guide", "md_README.html#autotoc_md32", [
          [ "Navigation (Panning and Zooming)", "md_README.html#autotoc_md33", null ],
          [ "Item Manipulation", "md_README.html#autotoc_md34", null ]
        ] ]
      ] ],
      [ "📚 Library Usage", "md_README.html#autotoc_md35", [
        [ "CMake Integration", "md_README.html#autotoc_md36", [
          [ "Method 1: Add as Subdirectory", "md_README.html#autotoc_md37", null ],
          [ "Method 2: Copy Headers", "md_README.html#autotoc_md38", null ]
        ] ],
        [ "Basic Usage Example", "md_README.html#autotoc_md39", null ],
        [ "Key Classes", "md_README.html#autotoc_md40", [
          [ "<tt>wxTimelineCtrl<T></tt>", "md_README.html#autotoc_md41", null ],
          [ "<tt>TimelineItemData</tt>", "md_README.html#autotoc_md42", null ],
          [ "<tt>TimelineArtProvider</tt>", "md_README.html#autotoc_md43", null ]
        ] ],
        [ "Events", "md_README.html#autotoc_md44", null ]
      ] ],
      [ "📖 Documentation", "md_README.html#autotoc_md45", [
        [ "API Documentation", "md_README.html#autotoc_md46", null ],
        [ "Documentation Features", "md_README.html#autotoc_md47", null ],
        [ "Generating Documentation Locally", "md_README.html#autotoc_md48", null ],
        [ "Documentation Updates", "md_README.html#autotoc_md49", null ]
      ] ],
      [ "🔧 Development", "md_README.html#autotoc_md50", [
        [ "CMake Targets", "md_README.html#autotoc_md51", null ],
        [ "Build Configuration", "md_README.html#autotoc_md52", [
          [ "Build Types", "md_README.html#autotoc_md53", null ],
          [ "Platform Support", "md_README.html#autotoc_md54", null ]
        ] ],
        [ "Custom Build Configuration", "md_README.html#autotoc_md55", null ]
      ] ],
      [ "🔄 Continuous Integration", "md_README.html#autotoc_md56", [
        [ "Supported Platforms", "md_README.html#autotoc_md57", null ],
        [ "Build Matrix", "md_README.html#autotoc_md58", null ],
        [ "Build Triggers", "md_README.html#autotoc_md59", null ],
        [ "Build Artifacts", "md_README.html#autotoc_md60", null ],
        [ "Build Process", "md_README.html#autotoc_md61", null ],
        [ "Local vs CI Build", "md_README.html#autotoc_md62", null ]
      ] ],
      [ "🛠️ Troubleshooting", "md_README.html#autotoc_md63", [
        [ "Common Issues", "md_README.html#autotoc_md64", [
          [ "1. WXWIN not set", "md_README.html#autotoc_md65", null ],
          [ "2. wxWidgets not found", "md_README.html#autotoc_md66", null ],
          [ "3. Build errors", "md_README.html#autotoc_md67", null ],
          [ "4. Missing executable", "md_README.html#autotoc_md68", null ]
        ] ],
        [ "Debug Build", "md_README.html#autotoc_md69", null ],
        [ "Verification Script", "md_README.html#autotoc_md70", null ]
      ] ],
      [ "📄 License", "md_README.html#autotoc_md71", null ],
      [ "💖 Support Open Source Development", "md_README.html#autotoc_md72", [
        [ "Why Support?", "md_README.html#autotoc_md73", null ],
        [ "Donate via PayPal", "md_README.html#autotoc_md74", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"FloatingItemPopupWindow_8h.html",
"classwxTimelineCtrl.html#abdbe3853bf5fb6c81768289b41365bba"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';