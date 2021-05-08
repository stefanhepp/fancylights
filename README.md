HomeControl
===========

This is the sourcecode and plans for my light and beamer control system.

Content
=======

- `keypad`: Sources for a keypad controller that communicates via RS232 with the main controller board and via I2C with a Raspberry PI.
- `lightcontrol`: Sources for the main light controller, which controls the lights and screen, and communicates via I2C with the beamer control board.
- `beamer`: Sources for the beamer control board, which controls a servo for the beamer mount locking mechanism and communicates via RS232 with the beamer.

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
