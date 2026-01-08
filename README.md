# ZMK-AUTO-LAYER

This module adds an `auto-layer` behavior to ZMK. A layer activated by the behavior continues to be
active for as long as keys in a configurable `continue-list` are pressed, and deactivates
automatically on any other key press.

This is a re-implementation of [PR #1451](https://github.com/zmkfirmware/zmk/pull/1451), separating
the `auto-layer` behavior from `caps-word` and making the layer index a parameter.

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
      revision: v0.3 # Set to desired ZMK release.
      import: app/west.yml
    - name: zmk-auto-layer
      remote: urob
      revision: v0.3 # Should match ZMK release.
  self:
    path: config
```

## Configuration

There are four configuration properties for the behavior:

- **`continue-list`** (required): An array of keycodes that will keep the layer active.
- **`ignore-alphas`** (optional): If set, the layer will not be deactivated by any alphabetic key.
- **`ignore-numbers`** (optional): If set, the layer will not be deactivated by any numeric key.
- **`ignore-modifiers`** (optional): If set, the layer will not be deactivated by any modifier key.

Behavior instances take one mandatory argument that specifies the index of the layer to be
activated.

## Example: Num-word

The module pre-configures a `num-word` behavior instance that activates a layer for as long as only
numbers and a few other keys are pressed. To use it, source the definition at the top your keymap:

```c
#include <behaviors/num_word.dtsi>
```

Then, add `&num_word NUM` anywhere to your keymap where `NUM` is the index of your numbers layer.

**Customization**: By default, `num_word` continues on number keys as well as on `BSPC`, `DEL`,
`DOT`, `COMMA`, `PLUS`, `MINUS`, `STAR`, `FSLH`, and `EQUAL`. To customize the `continue-list`,
overwrite it in the keymap. For instance:

```c
&num_word {
  continue-list = <BSPC DEL DOT COMMA>;
};
```

## Example: General case

Custom behavior instances can be defined using the general `auto-layer` behavior. The following
illustrates how to define a `nav-word` behavior that continues on arrow keys, `PG_UP`, `PG_DOWN`,
and all modifiers.

```c
/ {
  behaviors {
    nav_word: nav_word {
      compatible = "zmk,behavior-auto-layer";
      #binding-cells = <1>;
      continue-list = <LEFT DOWN UP RIGHT PG_DN PG_UP>;
      ignore-modifiers;
    };
  };
};
```

## References

- The behavior is inspired by Jonas Hietala's
  [Numword](https://www.jonashietala.se/blog/2021/06/03/the-t-34-keyboard-layout/#where-are-the-digits)
  for QMK
- A zero-parameter version where layers are part of the behavior definition
  is available [here](https://github.com/urob/zmk-auto-layer/tree/zero-param)
- My personal [zmk-config](https://github.com/urob/zmk-config) contains a more advanced example
