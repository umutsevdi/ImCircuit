# ImCircuit

A free and open-source cross-platform logic circuit simulator written in C++. 

> [!WARNING]  
> Currently in progress. Visit the original Logic Circuit Simulator from
> [here](https://github.com/umutsevdi/Logic-Circuit-Simulator).

https://github.com/user-attachments/assets/a770f99d-ed13-40cf-bca7-54b13cea11b8

## Roadmap

- Testing & Utilities
    - [X] Add tests for core library.
    - [X] Add tests for rest of testable methods.
    - [X] Test report generation.
    - [X] Testing pipeline
    - [X] Doxygen support
    - [X] Automated linux packaging 
    - [ ] Run tests on pipeline using generated package.
- Linux
    - [X] Create executable.
    - [X] Add packaging.
- Windows
    - [X] Create executable.
    - [X] Create installer.
    - [X] Reduce required permissions for the installer.
- Core Library
    - [X] Feature compatibility with in the logic circuit simulator.
    - [X] Implement serialization/deserialization.
    - [X] Indexed component context.
    - [X] Timer node execution on dependencies.
    - [X] Timer node execution on dependencies.
    - [ ] Refactor scene API to match with tabs API.
    - [ ] Implement undo mechanism.
- User Interface
    - [X] Upgrade to docking mode
    - [X] Save panel layout.
    - [X] Save positions of panels.
    - [X] Non-blocking native file dialogs.
    - [X] Theming options.
    - [X] Localization implementation.
    - [ ] Dependency explorer.
    - [ ] Package manager UI.
    - [ ] Profile viewer.
- Command Line Interface
    - [X] Add basic command line interface
    - [X] Implement a CLI shell with -i, --interactive arguments.
    - [X] Implement suggestions and hints.
- Network
    - [X] Implement networking utilities. 
    - [X] Implement async networking.
    - [X] Safe password storage.
    - [ ] Implement api handlers.
    - [ ] Develop a website to host packages.
        - [ ] Landing page
        - [ ] User profile pages
        - [ ] Package searching
        - [ ] Package uploading
        - [ ] API Endpoints
- Build System
    - [X] Windows DLL packaging with vcpkg.
    - [X] Add LCS_GUI option.
    - [X] Add LCS_PACKAGE target.
    - [X] Add bundle target.
- Localization
    - [X] Deutsch(de_DE)
    - [ ] Español(es_ES)
    - [ ] Français(fr_FR)
    - [X] 日本語(ja_JP)
    - [ ] Русский(ru_RU)
    - [X] Türkçe(tr_TR)
    - [ ] 中国人(zh_CN)
