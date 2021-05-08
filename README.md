ColorTroll
==========

This is the sourcecode and plans for my home LED light and projector control system.

Content
=======

This project consists of the following controller boards:
- `keypad`: Sources for a keypad controller that communicates via RS232 with the main controller board and via I2C with a Raspberry PI.
- `colortroll`: Sources for the main light controller, which controls the lights and screen, and communicates via I2C with the beamer control board.
- `projector`: Sources for the projector control board, which controls a servo for the projector mount locking mechanism and communicates via RS232 with the projector.

This repository contains the following folders:
- `kicad`: KiCAD schematics for all boards.
- `src`: Sources for all AtMega microcontrollers.

License
=======

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
