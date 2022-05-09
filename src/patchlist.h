#include <ArduinoJson.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include "setlist.h"

#define SPI_SPEED SD_SCK_MHZ(1)
#define CS_PIN 10

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1,  MIDI);

class Song {
  public:
    void addPatch(int p);
    int length();
    int numberOfPatches();
    int getPatch(int);
    void reset();
  private:
    int _length = 0;
    int _lastPatchIndex = -1;
    int _patches[20];
};

void Song::reset() {
  _lastPatchIndex = -1;
  _length = 0;
}

void Song::addPatch(int p) {
  _lastPatchIndex++;
  _patches[_lastPatchIndex] = p;
  _length++;
}
int Song::numberOfPatches() {
  return _length;
}
int Song::getPatch(int index) {
  if (index < 0) return _patches[0];
  else if (index > _lastPatchIndex) return _patches[_lastPatchIndex];
  else return _patches[index];
}

class Patchlist {
  public:
    Patchlist();
    void begin();
    void next();
    void prev();
    int getNumberOfSongs();
  private:
    void selectPatch();
    void loadSongNames();
    void loadCurrentSong();

    int _songPointer = 0;
    int _patchPointer = 0;
    Song _currentSong;
    char* _songNames[28];
    int _numberOfSongs = 0;
};

Patchlist::Patchlist() {
}


void Patchlist::begin() {
  MIDI.begin(4);

  loadSongNames();
  selectPatch();
}

int Patchlist::getNumberOfSongs() {
  return _numberOfSongs;
}

void Patchlist::loadSongNames() {
  StaticJsonDocument<16> setListFilter;
  setListFilter["setlist"] = true;

  DynamicJsonDocument doc = DynamicJsonDocument(2048);
  deserializeJson(doc, (const char*)setlist, DeserializationOption::Filter(setListFilter));
  serializeJsonPretty(doc, Serial);

  JsonArray jsonSetlist = doc["setlist"];
  int songIndex = 0;
  for(JsonVariant v : jsonSetlist) {
    _songNames[songIndex] = v.as<char*>();
    songIndex++;
    _numberOfSongs++;
    if (songIndex >= 28) {
      break;
    }
  }

  loadCurrentSong();
}

void Patchlist::next() {
  _patchPointer++;
  if (_patchPointer >= _currentSong.numberOfPatches()) {
    _patchPointer = 0;
    _songPointer++;
    loadCurrentSong();
  }
  if (_songPointer >= getNumberOfSongs()) {
    _songPointer = 0;
    _patchPointer = 0;
    loadCurrentSong();
  }
  selectPatch();
}

void Patchlist::prev() {
  _patchPointer--;
  if (_patchPointer < 0) {
    _songPointer--;
    loadCurrentSong();
    _patchPointer = _currentSong.numberOfPatches() - 1;
  }
  if (_songPointer < 0) {
    _songPointer = getNumberOfSongs() - 1;
    loadCurrentSong();
    _patchPointer = _currentSong.numberOfPatches() - 1;
  }
  selectPatch();
}


void Patchlist::loadCurrentSong() {
  StaticJsonDocument<256> songFilter;// = DynamicJsonDocument(128);
  const char* name = _songNames[_songPointer];
  //Serial.println(name);
  songFilter["songs"][name] = true;

  StaticJsonDocument<2048> doc;// = DynamicJsonDocument(2048);
  deserializeJson(doc, (const char*) setlist, DeserializationOption::Filter(songFilter));
  serializeJsonPretty(doc, Serial);

  _currentSong.reset();
  JsonArray pl = doc["songs"][_songNames[_songPointer]];
  for (JsonVariant p : pl) {
    _currentSong.addPatch(p.as<int>());
  }
  doc.clear();
}

void Patchlist::selectPatch() {
  Serial.println(_currentSong.getPatch(_patchPointer));
  int rawPc = _currentSong.getPatch(_patchPointer) - 1;
  int bank = rawPc / 125;
  int pc = rawPc - bank * 125;
  
  // send a bank change
  MIDI.sendControlChange(0, bank, 1);
  // finish the control change
  MIDI.sendControlChange(20, 0, 1);
  MIDI.sendProgramChange((byte) (pc), 1);
}



