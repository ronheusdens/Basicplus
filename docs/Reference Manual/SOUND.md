# SOUND Statement

**BASIC Set:** Extended (SDL2 Audio Support)

## Syntax (Traditional)
```
SOUND frequency, duration
```

## Syntax (With Harmonics)
```
SOUND base_frequency; harmonic_1, intensity_1 [; harmonic_2, intensity_2 ...]; duration
```

## Parameters

### Traditional Format
- **frequency**: A numeric expression specifying the frequency in Hz (Hertz)
  - Valid range: 20 to 4000 Hz
  - Values outside this range are clamped to the valid range
  
- **duration**: A numeric expression specifying the duration in milliseconds
  - Valid range: 0 to 5000 ms
  - Values outside this range are clamped to the valid range

### Harmonics Format
- **base_frequency**: Fundamental frequency in Hz (20-4000)
- **harmonic_N**: Harmonic number (1 = fundamental, 2 = first overtone, 3 = second overtone, etc.)
  - The frequency played for each harmonic is: `base_frequency * harmonic_number`
  
- **intensity_N**: Relative intensity of the harmonic (0.0 to 1.0)
  - 0.0 = silent  
  - 1.0 = full volume
  - Values outside 0-1 range are clamped
  - Multiple harmonics are summed and normalized to prevent clipping
  
- **duration**: Total duration in milliseconds (0-5000)

## Description

The SOUND statement produces a tone at the specified frequency (or frequencies with harmonics) for a specified duration using the system's audio capabilities. The implementation uses SDL2's audio system for cross-platform compatibility, with a fallback to `afplay` on macOS if SDL audio is unavailable.

SOUND generates one or more sine waves that can be combined to create richer, more complex tones.

Uses:
- **Traditional**: Simple tones and beeps for alerts
- **Harmonics**: Musical notes with instrument-like qualities, better approximation of real sounds

## Comparison with BEEP

| Feature | SOUND (Traditional) | SOUND (Harmonics) | BEEP |
|---------|----|----|------|
| Syntax | SOUND freq, dur | SOUND base; h,i; dur | BEEP [dur [, tone]] |
| Frequency Control | Single frequency | Multiple harmonics | Limited (LOW/MID/HIGH) |
| Sound Quality | Pure sine wave | Rich harmonic content | System default |
| Musical Capability | Low | High | Very limited |

## Examples

### Traditional SOUND Examples

#### Basic Tones
```basic
10 PRINT "Playing different frequencies"
20 SOUND 220, 500  ' A3 note - 500ms
30 SOUND 440, 500  ' A4 note (concert pitch)
40 SOUND 880, 500  ' A5 note
50 END
```

#### Simple Melody
```basic
10 REM Simple musical scale
20 SOUND 262, 200  ' C
30 SOUND 294, 200  ' D
40 SOUND 330, 200  ' E
50 SOUND 349, 200  ' F
60 SOUND 392, 200  ' G
70 END
```

#### Alert Tones
```basic
10 INPUT "Enter a number: ", N
20 IF N < 0 THEN SOUND 800, 300 : PRINT "Error!"
30 SOUND 440, 200 : PRINT "OK"
40 END
```

### Harmonics SOUND Examples

#### Basic Harmonics (Richer Tone)
```basic
10 PRINT "Playing A4 with harmonics"
20 REM Base frequency: 440 Hz (A4)
30 REM Add 1st and 3rd harmonics for warmth
40 SOUND 440; 1, 1; 3, 0.5; 800
50 END
```

#### Instrument-like Tone (Violin-like)
```basic
10 PRINT "Violin-like tone"
20 REM Base + strong 2nd and 3rd harmonics
30 SOUND 330; 1, 1; 2, 0.8; 3, 0.6; 5, 0.3; 1000
40 END
```

#### Clarinet-like Tone (Odd Harmonics Dominant)
```basic
10 PRINT "Clarinet-like tone"
20 REM Clarinet emphasizes odd harmonics
30 SOUND 262; 1, 1; 3, 0.7; 5, 0.4; 7, 0.2; 800
40 END
```

#### Complex Harmonic Sweep
```basic
10 FOR BASE = 200 TO 800 STEP 100
20   REM Create harmonically rich sweep
30   SOUND BASE; 1, 1; 2, 0.6; 3, 0.4; 300
40   PRINT "Frequency: "; BASE; " Hz"
50 NEXT BASE
60 END
```

#### Frequency Loop
```basic
10 PRINT "Frequency sweep"
20 FOR F = 200 TO 2000 STEP 100
30   SOUND F, 100
40 NEXT F
50 PRINT "Done"
60 END
```

## Audio Implementation

The interpreter uses SDL2's audio subsystem for sound generation:
- **Sample Rate**: 44100 Hz
- **Format**: 16-bit signed mono
- **Harmonics Algorithm**: Sine wave summation with normalization
- **Fallback**: On macOS, if SDL audio unavailable, generates WAV and plays with `afplay`

## Notes

- The SOUND statement blocks execution while the tone plays. Program execution resumes after the duration expires.
- The frequency range is limited for audio quality and system compatibility.
- Very short durations (< 10ms) may not be audible.
- On systems without audio support, the statement executes silently.
- Multiple SOUND statements in sequence create musical effects.
- Harmonic numbers can be any positive integer; frequencies above 20kHz are clamped.
- All harmonics in a statement are summed and then normalized to prevent audio clipping.
- At least one harmonic must have non-zero intensity, or the tone will be silent.

## Typical Note Frequencies (Hz)

```
C3: 131    C4: 262    C5: 523    C6: 1047
D3: 147    D4: 294    D5: 587    D6: 1175
E3: 165    E4: 330    E5: 659    E6: 1319
F3: 175    F4: 349    F5: 698    F6: 1397
G3: 196    G4: 392    G5: 784    G6: 1568
A3: 220    A4: 440    A5: 880    A6: 1760
B3: 247    B4: 494    B5: 988    B6: 1976
```

## Harmonic Series Reference

For a base frequency F:
- Harmonic 1: F (fundamental) - most natural
- Harmonic 2: 2F (octave up) - pure, consonant
- Harmonic 3: 3F (perfect fifth + octave) - sweet, warm
- Harmonic 4: 4F (two octaves up) - bright
- Harmonic 5: 5F (major third + 2 octaves) - bright, complex
- Harmonics 6+: Higher frequencies, increasingly dissonant

**Typical Intensities for Instrument Simulation:**
- **Sine Wave** (purest): 1; (nothing else)
- **Square Wave** (hollow): 1, 0; 3, 0.33; 5, 0.2; 7, 0.14
- **Violin**: 1, 1; 2, 0.8; 3, 0.6; 4, 0.4; 5, 0.2
- **Clarinet**: 1, 1; 3, 0.7; 5, 0.4; 7, 0.2 (odd harmonics only)
- **Flute**: 1, 1; 2, 0.3; 3, 0.2 (mostly fundamental)
- **Bell**: 1, 0.3; 2, 0.5; 3, 1; 4, 0.8; 5, 0.6 (emphasis on midrange)

## Related Statements
- BEEP - Produces a simple audible beep
- SLEEP - Pause execution without sound
