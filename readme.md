// (very) short version

This project was built for an embedded-systems course at FH Technikum Wien.

It uses three TI TivaBoards and max4466 microphone amplifiers to measure acustic signals. 
Once a threshold is reached the corresponding TivaBoard sends a timestamp-message to a server (Raspberry Pi).
If the server receives timestamp-messages from all three TivaBoards it calculates which TivaBoard was the closest to the sound signal
thus, the first to send a message. The result is shown on a simple webpage in the standard-webbrowser of the server.
