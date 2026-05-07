                        CHESS GAME

# Problem Statement:

The problem was to create a chess game using OOP concepts like: 

• Encapsulation
• Inheritance
• Polymorphism
• Composition

# Description:

# C++ SFML Chess Engine

A high-performance 2D Chess engine built with **C++** and the **SFML** (Simple and Fast Multimedia Library). This project focuses on clean, object-oriented design and efficient game logic, featuring a custom-built coordinate mapping system to bridge the gap between graphical pixel input and a classic 8x8 grid.

### Key Features
*   **Object-Oriented Architecture:** Modular classes for the board, pieces, and game engine for easy scalability.
*   **Full Rule Implementation:** Support for standard movement, collision detection, and specialized chess rules like castling and promotion.
*   **Intuitive GUI:** A responsive interface using SFML sprites and textures for smooth piece interaction.
*   **Standard Chess Logic:** Built-in move validation and turn management to ensure a rigorous adherence to FIDE rules.

### Tech Stack
*   **Language:** C++17
*   **Graphics:** SFML (Simple and Fast Multimedia Library) Version: 3.6.1
*   **Version Control:** GitHub
*   **Development Environment:** Visual Studio


# Technical Documentation: Chess Engine Implementation
**Course:** [Insert Course Name, e.g., Object-Oriented Programming]
**Developer:** [Your Name]
**Technology Stack:** C++, SFML (Simple and Fast Multimedia Library)

---

## 1. Project Overview
This project is a desktop-based Chess application developed using C++. It utilizes the SFML library for hardware-accelerated 2D graphics and event handling. The primary focus of the project was to implement a complex rule-based system using Object-Oriented Programming (OOP) principles.

## 2. System Architecture
The application is built on a modular architecture to ensure separation of concerns between game logic and the graphical user interface (GUI).

### Core Components:
*   **The Logic Engine:** Manages an $8 \times 8$ matrix representing the board. It handles turn-based state machine logic and move validation.
*   **The Rendering Layer:** Maps the internal logic state to the screen. It translates board indices $[i][j]$ into screen coordinates $(x, y)$ for sprite placement.
*   **Event Handler:** Captures mouse input and translates screen-space clicks into board-space interactions.

## 3. Object-Oriented Design
To manage the complexity of Chess, the project utilizes several OOP design patterns:

*   **Inheritance:** A base `Piece` class defines common attributes (color, position, texture). Specific pieces (Pawn, Knight, etc.) inherit from this and override the `isValidMove()` function.
*   **Encapsulation:** Board state and piece positions are encapsulated within the `Board` class, accessible only through defined public methods.
*   **Polymorphism:** The game loop iterates through a collection of `Piece` pointers, calling movement logic dynamically based on the object type.

## 4. Key Technical Features
| Feature | Implementation Detail |
| :--- | :--- |
| **Move Validation** | Algorithms to check for sliding path obstructions (Rooks/Bishops) and unique movement patterns (Knights). |
| **Coordinate Mapping** | Direct transformation of pixel-based mouse coordinates to discrete board grid indices. |
| **Texture Management** | Centralized loading of graphical assets to optimize memory usage and prevent redundant I/O operations. |
| **Turn Management** | A strictly enforced state-switch logic ensuring legal play sequence. |

## 5. Development Workflow
*   **Version Control:** Managed via **Git/GitHub** to track feature branches and maintain code integrity.
*   **IDE:** Developed and debugged using **Visual Studio**, utilizing its native C++ compiler and debugger tools.
*   **Build System:** Linked with SFML static/dynamic libraries for cross-platform potential.

## 6. Advanced Game Logic & Features
To achieve a professional standard of play, the following advanced chess mechanics were implemented:

### A. Special Move: Castling
*   **Logic:** A simultaneous move involving the King and Rook. 
*   **Constraints:** The system verifies three conditions:
    1. Neither the King nor the target Rook has moved previously in the game.
    2. All squares between the King and Rook are currently unoccupied.
    3. The King is not currently in check, nor does it pass through a square controlled by an opponent's piece.

### B. Endgame States: Checkmate & Resignation
*   **Checkmate Detection:** Triggered when a King is in "Check" and the `Validator` determines there are zero legal moves remaining for any piece of that color to remove the threat.
*   **Resignation:** A GUI-driven feature allowing a player to concede the match instantly, terminating the game loop and declaring the opponent the winner.

### C. Game Conclusion: Offering a Draw
*   **Mutual Consent:** Implemented as a request-response toggle. One player proposes a draw; the other can accept via the UI, leading to a `STALEMATE` or `DRAW` state regardless of the board's material advantage.

### D. Pawn Promotion
*   **Logic:** When a `Pawn` object reaching the 0th or 7th rank (depending on color), a UI modal appears.
*   **Technical Execution:** The existing `Pawn` object is deallocated, and a new `Queen`, `Rook`, `Bishop`, or `Knight` object is dynamically instantiated in its place.

---

## 7. User Interface (UI) Elements
The SFML-based interface was expanded to include:
*   **Status Bar:** Real-time text feedback (e.g., "White's Turn", "Check!", or "Draw Offered").
*   **Control Panel:** Dedicated buttons for 'Resign' and 'Offer Draw' handled through SFML button-collision detection.
*   **Visual Highlights:** Last move highlighting and valid move indicators to improve user experience (UX).


## 8. Conclusion & Future Scope
The current iteration successfully implements all fundamental chess movements and visual feedback. Future versions aim to include:
*   **Advanced Logic:** Castling, En Passant, and Pawn Promotion.
*   **AI Integration:** Implementation of the Minimax algorithm for single-player functionality.
*   **Networking:** Socket-based multiplayer using `sfml-network`.
