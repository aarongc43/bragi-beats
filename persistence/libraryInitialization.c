#include "libraryInitialization.h"

void initializeLibrary(const char* userPath);
void AddAlbumToLibrary(const char* albumName);
void AddSongToAlbum(const char* albumName, const char* songName);

void initializeLibrary(const char* userPath) {
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(userPath)) == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry ->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            AddAlbumToLibrary(entry->d_name);
        } else if (entry->d_type == DT_REG) {
            AddSongToAlbum("Root", entry->d_name);
        }
    }
    closedir(dir);
}

void AddAlbumToLibrary(const char* albumName) {
    AddSongToAlbum(albumName, NULL);
}

void AddSongToAlbum(const char* albumName, const char* songName) {
    Album *currentAlbum = userLibrary.albums;

    while (currentAlbum != NULL && strcmp(currentAlbum->name, albumName) != 0) {
        currentAlbum = currentAlbum->next;
    }

    if (currentAlbum == NULL) {
        Album *newAlbum = malloc(sizeof(Album));
        strcpy(newAlbum->name, albumName);
        newAlbum->songs = NULL;
        newAlbum->next = userLibrary.albums;
        userLibrary.albums = newAlbum;
        userLibrary.albumCount++;
        currentAlbum = newAlbum;
    }

    Song *newSong = malloc(sizeof(Song));
    strcpy(newSong->name, songName);
    newSong->next = currentAlbum->songs;
    currentAlbum->songs = newSong;
}
