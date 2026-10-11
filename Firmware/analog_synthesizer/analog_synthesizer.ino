
#include <SPI.h>

const int CS_PIN = 10;
const int GATE_PIN = 2;

const float BASE_CV = 1.024f;
const float VOLTS_PER_OCTAVE = 0.612f;

byte heldNotes[128];
int heldCount = 0;

void setDAC(uint16_t value) {
  value &= 0x0FFF;
  uint16_t command = 0x3000 | value;

  SPI.beginTransaction(
    SPISettings(1000000, MSBFIRST, SPI_MODE0)
  );

  digitalWrite(CS_PIN, LOW);
  SPI.transfer16(command);
  digitalWrite(CS_PIN, HIGH);

  SPI.endTransaction();
}

void playNote(byte note) {
  float cv = BASE_CV +
             (int(note) - 60) * VOLTS_PER_OCTAVE / 12.0f;

  cv = constrain(cv, 0.0f, 2.0475f);

  uint16_t dacCode = (uint16_t)(cv * 2000.0f + 0.5f);

  setDAC(dacCode);
  digitalWrite(GATE_PIN, HIGH);
}

void removeHeldNote(byte note) {
  for (int i = 0; i < heldCount; i++) {
    if (heldNotes[i] == note) {
      for (int j = i; j < heldCount - 1; j++) {
        heldNotes[j] = heldNotes[j + 1];
      }
      heldCount--;
      return;
    }
  }
}

void onNoteOff(byte channel, byte note, byte velocity) {
  removeHeldNote(note);

  if (heldCount == 0) {
    digitalWrite(GATE_PIN, LOW);
  } else {
    playNote(heldNotes[heldCount - 1]);
  }
}

void onNoteOn(byte channel, byte note, byte velocity) {
  if (velocity == 0) {
    onNoteOff(channel, note, 0);
    return;
  }

  removeHeldNote(note);
  heldNotes[heldCount++] = note;
  playNote(note);
}

void setup() {
  digitalWrite(GATE_PIN, LOW);
  pinMode(GATE_PIN, OUTPUT);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  SPI.begin();
  setDAC(2048);

  usbMIDI.setHandleNoteOn(onNoteOn);
  usbMIDI.setHandleNoteOff(onNoteOff);
}

void loop() {
  while (usbMIDI.read()) {
  }
}
