# ZMK-AUTO-LAYER

This module adds an `auto-layer` behavior to ZMK. A layer activated by the behavior continues to be
active for as long as keys in a configurable `continue-list` are pressed, and deactivates
automatically on any other key press.

This is a re-implementation of [PR #1451](https://github.com/zmkfirmware/zmk/pull/1451), separating
the `auto-layer` behavior from `caps-word`. As the behavior does not require any modifications of
ZMK's core api, it can be implemented as a module.

## Usage

To load the module, add the following entries to `remotes` and `projects` in `config/west.yml`.

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: urob
      url-base: https://github.com/urob
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    - name: zmk-auto-layer
      remote: urob
      revision: main
  self:
    path: config
```

## Configuration

The behavior takes five properties:

- **`layers`** (required): An array of layer indices to activate when the behavior is triggered.
- **`continue-list`** (required): An array of keycodes that will keep the layer active.
- **`ignore-alphas`** (optional): If set, the layer will not be deactivated by any alphabetic key.
- **`ignore-numbers`** (optional): If set, the layer will not be deactivated by any numeric key.
- **`ignore-modifiers`** (optional): If set, the layer will not be deactivated by any modifier key.

## Example: Numword

The following illustrates how `auto-layer` can be used to implement a `num-word` behavior that
activates a numbers layer for as long as only numbers and a few other keys are pressed.

```dts
/ {
  behaviors {
    num_word: num_word {
      compatible = "zmk,behavior-auto-layer";
      #binding-cells = <0>;
      layers = <NUM>; // Replace NUM by index of numbers layer
      continue-list = <BSPC DEL DOT COMMA PLUS MINUS STAR FSLH EQUAL>;
      ignore-numbers;
    };
  };
};
```
