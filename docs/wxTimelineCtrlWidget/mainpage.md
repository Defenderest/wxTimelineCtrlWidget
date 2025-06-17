# wxTimelineCtrl API Reference

Welcome to the official API reference documentation for the `wxTimelineCtrl` component.

This reference provides a detailed, technical overview of all the classes, methods, and data structures that constitute the component. It is intended for developers who wish to integrate `wxTimelineCtrl` into their own projects or contribute to its development.

## What is wxTimelineCtrl?

`wxTimelineCtrl` is a flexible and interactive C++ control for the wxWidgets framework, designed for visualizing time-based data. It is ideally suited for applications such as:

*   Video and audio editors
*   Project management tools with Gantt charts
*   Event schedulers and data log visualizers

## Key Architectural Features

*   **Templated Design:** The control is a template (`template<typename T>`), allowing it to work with any user-defined data structure that inherits from `TimelineItemData`.
*   **Interactive Controls:** It supports smooth zooming, panning, as well as item dragging and resizing.
*   **Dual-View Navigation:** It features a detailed main timeline view and a full-range scroller for quick navigation across the entire time span.
*   **Customizable Drawing:** The entire look and feel can be customized by inheriting from the `TimelineArtProvider` class.
*   **Event-Driven Architecture:** It integrates easily with application logic through a rich set of `wxCommandEvent`s.

## How to Use This Documentation

To work effectively with this reference, we recommend exploring the following sections:

- The **Class** `wxTimelineCtrl` is the main entry point and primary class of the component. Its documentation contains a "quick start" code example for basic setup.
- The **Classes** tab in the top navigation bar provides a complete alphabetical list of all classes in the library.
- The **Files** tab provides access to the documentation for each source file, which is useful for understanding the project's structure.


