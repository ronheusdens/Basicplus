# Instrument Harmonics Reference

This document details the harmonic content needed to simulate various orchestral and acoustic instruments using the SOUND statement with harmonics.

## Violin Harmonics

**Characteristics:** Rich, warm, sustaining tone with emphasis on mid-range harmonics

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 2, 0.8; 3, 0.6; 4, 0.4; 5, 0.2; 500
```

**Harmonic Breakdown:**
- **H1 (1.0)**: Fundamental - pure, warm base
- **H2 (0.8)**: Octave - adds brightness and projection
- **H3 (0.6)**: Perfect 12th - rich warmth
- **H4 (0.4)**: Two octaves - sparkle and shimmer
- **H5 (0.2)**: Major 17th - subtle complexity

**Use Case:** Orchestral strings, bowed instruments, lyrical passages

---

## Cello Harmonics

**Characteristics:** Deeper, richer than violin; emphasis on lower harmonics

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 2, 0.9; 3, 0.7; 4, 0.3; 5, 0.1; 600
```

**Harmonic Breakdown:**
- **H1 (1.0)**: Strong fundamental
- **H2 (0.9)**: Very prominent octave
- **H3 (0.7)**: Rich warmth
- **H4 (0.3)**: Subtle brightness
- **H5 (0.1)**: Minimal complexity

**Use Case:** Deep, resonant passages; bass lines

---

## Flute Harmonics

**Characteristics:** Pure, airy tone; mostly fundamental with gentle overtones

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 2, 0.3; 3, 0.15; 400
```

**Harmonic Breakdown:**
- **H1 (1.0)**: Near-pure fundamental
- **H2 (0.3)**: Slight shimmer
- **H3 (0.15)**: Minimal overtone

**Use Case:** Woodwind instruments, clear melodic lines

---

## Clarinet Harmonics

**Characteristics:** Hollow, woody tone; ODD harmonics only (natural clarinet behavior)

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 3, 0.7; 5, 0.4; 7, 0.2; 500
```

**Harmonic Breakdown:**
- **H1 (1.0)**: Strong fundamental
- **H3 (0.7)**: Strong odd harmonic (characteristic reedy quality)
- **H5 (0.4)**: Brightness
- **H7 (0.2)**: Subtle complexity

**Note:** Clarinet naturally emphasizes odd harmonics only due to its bore design

**Use Case:** Woodwind sections, solo reeds

---

## Oboe Harmonics

**Characteristics:** Bright, penetrating, nasal quality

**Formula:**
```basic
SOUND base_freq; 1, 0.9; 2, 0.8; 3, 0.9; 4, 0.5; 5, 0.3; 450
```

**Harmonic Breakdown:**
- **H1 (0.9)**: Strong fundamental
- **H2 (0.8)**: Prominent octave
- **H3 (0.9)**: Very strong (creates nasal quality)
- **H4 (0.5)**: Brightness
- **H5 (0.3)**: Complexity

**Use Case:** Piercing melodic lines, solo passages

---

## French Horn Harmonics

**Characteristics:** Warm, mellow, veiled tone

**Formula:**
```basic
SOUND base_freq; 1, 0.8; 2, 0.7; 3, 0.9; 4, 0.6; 5, 0.4; 6, 0.2; 550
```

**Harmonic Breakdown:**
- Complex blend emphasizing H2 and H3 equally
- Creates warm, soft sonority

**Use Case:** Lyrical passages, background harmony

---

## Trumpet Harmonics

**Characteristics:** Brilliant, penetrating, brassy

**Formula:**
```basic
SOUND base_freq; 1, 0.7; 2, 0.9; 3, 1.0; 4, 0.8; 5, 0.5; 6, 0.3; 400
```

**Harmonic Breakdown:**
- **H1 (0.7)**: Moderate fundamental
- **H2 (0.9)**: Strong octave
- **H3 (1.0)**: PEAK brightness (defines brassy quality)
- **H4 (0.8)**: Strong presence
- **H5 (0.5)**: Harshness
- **H6 (0.3)**: Shimmer

**Use Case:** Fanfares, brilliant passages, piercing melodies

---

## Trombone Harmonics

**Characteristics:** Warm, full, smooth brass tone

**Formula:**
```basic
SOUND base_freq; 1, 0.8; 2, 0.8; 3, 0.9; 4, 0.6; 5, 0.4; 6, 0.2; 600
```

**Harmonic Breakdown:**
- Strong fundamental and octave
- H3 peak creates warm brass character
- Slightly less bright than trumpet

**Use Case:** Lyrical brass passages, full ensemble harmony

---

## Pipe Organ (Open Pipe - Diapason)

**Characteristics:** Full, warm, smooth

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 2, 0.8; 3, 0.6; 4, 0.4; 5, 0.2; 6, 0.1; 800
```

**Harmonic Breakdown:**
- Even-weighted harmonic series
- Open pipe has all harmonics present
- Smooth, singing quality

**Use Case:** Background texture, sustained chords

---

## Struck Bell

**Characteristics:** Complex, inharmonic-like, emphasis on midrange

**Formula:**
```basic
SOUND base_freq; 1, 0.3; 2, 0.5; 3, 1.0; 4, 0.8; 5, 0.6; 6, 0.4; 7, 0.2; 2000
```

**Harmonic Breakdown:**
- De-emphasized fundamental (bells have weak fundamentals)
- **H3 PEAK**: Bell characteristic
- Long decay simulated with longer duration

**Use Case:** Bells, gongs, percussion effects; use 2000+ ms duration

---

## Piano Harmonics

**Characteristics:** Depends on octave; upper registers brighter, lower registers mellower

**Formula (Upper Octave - Bright):**
```basic
SOUND base_freq; 1, 0.9; 2, 0.8; 3, 0.7; 4, 0.6; 5, 0.5; 6, 0.3; 800
```

**Formula (Lower Octave - Mellow):**
```basic
SOUND base_freq; 1, 1.0; 2, 0.7; 3, 0.4; 4, 0.2; 1200
```

**Use Case:** Piano textures, sustained notes; vary duration based on "pedal"

---

## Guitar Harmonics

**Characteristics:** Bright, percussive attack then quick decay

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 2, 0.6; 3, 0.4; 4, 0.3; 5, 0.2; 300
```

**Harmonic Breakdown:**
- Clean fundamental
- Moderate overtones for "twang"
- Short duration recommended (plucked effect)

**Use Case:** Plucked strings, bright passages; use 200-400 ms

---

## Electric Bass Harmonics

**Characteristics:** Deep, punchy with slight edge

**Formula:**
```basic
SOUND base_freq; 1, 1.0; 2, 0.7; 3, 0.3; 4, 0.2; 5, 0.1; 700
```

**Harmonic Breakdown:**
- Dominant fundamental
- Minimal upper harmonics for clarity
- Clean, modern tone

**Use Case:** Bass lines, low frequency passages

---

## Examples

### Implementation in BASIC

```basic
10 REM Function definitions for common instruments
20 REM Call with: GOSUB 1000 (violin), 1020 (flute), etc.

100 REM Play note with violin timbre
110 SOUND 440; 1, 1; 2, 0.8; 3, 0.6; 4, 0.4; 5, 0.2; 500
115 RETURN

200 REM Play note with clarinet timbre
210 SOUND 440; 1, 1; 3, 0.7; 5, 0.4; 7, 0.2; 500
215 RETURN

300 REM Play note with flute timbre
310 SOUND 440; 1, 1; 2, 0.3; 3, 0.15; 400
315 RETURN

999 END

1000 REM VIOLIN subroutine
1010 REM Expects BASE_FREQ and DUR variables
1020 SOUND BASE_FREQ; 1, 1; 2, 0.8; 3, 0.6; 4, 0.4; 5, 0.2; DUR
1030 RETURN
```

### Orchestra Simulation

```basic
10 REM Simple orchestra chord
20 BASE = 262  ' C4

30 REM Violin melody
40 SOUND BASE*1.5; 1, 1; 2, 0.8; 3, 0.6; 4, 0.4; 5, 0.2; 400

50 REM (These would play simultaneously in a real system)
60 REM Cello harmony (lower)
70 REM SOUND BASE; 1, 1; 2, 0.9; 3, 0.7; 4, 0.3; 5, 0.1; 400

80 REM Flute counter-melody
90 REM SOUND BASE*2; 1, 1; 2, 0.3; 3, 0.15; 400

100 END
```

---

## Tips for Best Results

1. **Duration**: Longer durations (500-1000ms) sound more natural
2. **Intensity Balance**: Keep H1 at 1.0; lower harmonics by 0.1-0.2 each step
3. **Brightness**: Brighter instruments have stronger higher harmonics
4. **Resonance**: Deeper instruments emphasize H2 and H3 equally
5. **Character**: The peak harmonic (highest intensity) defines the instrument character
6. **Realism**: Very complex harmonic patterns (7+ harmonics) can sound more realistic

---

## Harmonic Series Guide

```
Harmonic | Interval from Fundamental | Ratio | Frequency (if base=A3:220Hz)
---------|---------------------------|-------|---------------------------
   1     | Unison                    | 1:1   | 220 Hz
   2     | Octave                    | 2:1   | 440 Hz
   3     | Perfect 12th (Octave+5th) | 3:1   | 660 Hz
   4     | 2 Octaves                 | 4:1   | 880 Hz
   5     | Major 17th                | 5:1   | 1100 Hz
   6     | 2 Octaves + Major 3rd     | 6:1   | 1320 Hz
   7     | Minor 7th + 2 Octaves     | 7:1   | 1540 Hz
   8     | 3 Octaves                 | 8:1   | 1760 Hz
   9     | Major 2nd + 3 Octaves     | 9:1   | 1980 Hz
```

---

## Creating Custom Instruments

**Process:**
1. Identify the dominant characteristic (bright, dark, hollow, warm)
2. Find the harmonic peak (highest intensity) - this defines character
3. Taper harmonics smoothly (avoid abrupt changes)
4. Match natural instrument harmonic series where possible
5. Experiment with duration and intensity ratios

**Typical Patterns:**
- **Bright**: Peak on H3-H4
- **Dark**: Peak on H2 or emphasize H1-H2
- **Hollow**: Weak H1, strong H3
- **Warm**: Balanced H1-H4 with gradual falloff
- **Harsh**: Multiple peaks or discontinuities
