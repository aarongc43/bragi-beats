Actors
User: The primary actor who interacts with the music visualizer application.
System: Represents the music visualizer application itself, including its components like the music player, visualization engine, and database.
Use Cases
1. Authenticate User
Actor: User
Description: The user logs into the application using their username and password. The system validates the credentials against the database.
Steps:
User opens the application and is prompted to log in.
User enters username and password.
System validates the credentials.
User gains access to the application on successful validation.
2. Load Music File
Actor: User
Description: The user drags and drops a music file into the application. The system loads the file and prepares it for playback.
Steps:
User selects a music file and drags it into the application window.
System verifies the file format and loads the file.
System updates the user's library with the new song.
3. Play Music
Actor: User
Description: The user plays a selected music file. The system reads the file, processes the audio, and starts playback.
Steps:
User selects a song from their library or playlist.
User presses the play button.
System processes and plays the music.
4. Visualize Music
Actor: System
Description: As music plays, the system generates and displays visualizations corresponding to the audio output.
Steps:
System analyzes the audio data using FFT.
System generates visualization based on the audio data.
Visualization is displayed in sync with the music.
5. Manage Playlists
Actor: User
Description: The user creates, updates, or deletes playlists. The system updates the database accordingly.
Steps:
User chooses to create a new playlist, add songs to an existing playlist, or delete a playlist.
System updates the playlists in the database based on user actions.
6. Adjust Settings
Actor: User
Description: The user adjusts application settings, including visualization preferences, audio output settings, and account details.
Steps:
User opens the settings menu.
User makes changes to the settings.
System applies and saves the new settings.
Relationships and Extensions
Load Music File extends Authenticate User, as the user must be authenticated before they can load music files.
Play Music and Visualize Music are closely related; Visualize Music cannot happen without Play Music.
Manage Playlists includes adding songs to playlists, where Load Music File is a prerequisite step for adding new songs to the system.
