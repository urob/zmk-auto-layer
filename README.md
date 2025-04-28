# English

Module `ruen` lets you send keycodes regardless of the active system language or keyboard layout. All examples and explanations use Russian and English layouts, but you can apply the same approach to any other language pair, for example French and English or Spanish and English.

> [!WARNING]  
> This module is currently in early alpha. Use at your own risk.

## New behaviors

- `&ruen_switch <0/1> <hotkey>` — switch both the system layout and the internal keyboard layout according to your configuration:  
  - `0`: internal layout → Russian  
  - `1`: internal layout → English  
  - `hotkey`: the system‐level shortcut to change the layout  

- `&ruen_key <en_key> <ru_key>` — send `en_key` if the active language is English, or `ru_key` if it’s Russian:  
  - `en_key`: keycode to send in English layout  
  - `ru_key`: keycode to send in Russian layout  

- `&ruen_one_key <0/1> <key>` — if the current language matches your configuration, send `key`; otherwise, perform: switch → send `key` → switch back:  
  - `0`: priority → Russian  
  - `1`: priority → English  
  - `key`: the keycode to send  

## Examples

> [!NOTE]  
> Examples assume macOS with English and Russian layouts.

- `&ruen_switch 0 LG(N9)` — internal keyboard switches to Russian; sends `LGui+9` to switch the system to Russian.  
- `&ruen_switch 1 LG(N8)` — internal keyboard switches to English; sends `LGui+8` to switch the system to English.  
- `&ruen_key LS(N5) LS(N4)` — in English layout sends `LShift+5` (prints `%`), in Russian layout sends `LShift+4` (also prints `%`). Always yields `%`.  
- `&ruen_key Q W` — in English layout sends `Q`, in Russian layout sends `W` (Ц). Lets you combine Colemak and the “Dictor” (Russian alternative layout) layouts in one layer without changing system settings.  
- `&ruen_one_key 1 LS(N3)` — in English layout sends `LShift+3` (`#`); in Russian: switch to English → `LShift+3` → switch back. Always yields `#`.  
- `&ruen_one_key 0 LS(N3)` — in Russian layout sends `LShift+3` (`№`); in English: switch to Russian → `LShift+3` → switch back. Always yields `№`.  

[More examples in my keymap](https://github.com/jewtvet-org/zmk-config-new/blob/main/config/cradio.keymap)

## Usage on MacOS

1. Install **MLSwitcher2** from Mac AppStore and configure separate hotkeys for Russian and English layouts.  
2. In your `config/west.yml`, add these entries under `remotes` and `projects`:
   ```yaml
   manifest:
     remotes:
       - name: zmkfirmware
         url-base: https://github.com/zmkfirmware
       - name: jewtvet
         url-base: https://github.com/jewtvet
     projects:
       - name: zmk
         remote: zmkfirmware
         revision: main
         import: app/west.yml
       - name: zmk-ruen
         remote: jewtvet
         revision: main
     self:
       path: config
   ```
3. In your `config/<your-keyboard>.keymap`, under `behaviors`, add these entries, update `to_en` and `to_ru` to your own layout-switch hotkeys:
   ```yaml
   ruen_one_key: ruen_one_key {
     compatible = "zmk,behavior-ruen-one-key";
     #binding-cells = <2>;
     to_en = <0x8070025>;  # int code for your hotkey to switch to English (in this example: LG(N8))
     to_ru = <0x8070026>;  # int code for your hotkey to switch to Russian (in this example: LG(N9))
   };
   ```
4. Add your new keys into the same keymap file.
5. Add `&ruen_macos 1` to a distant key position in a rarely used layer.
6. Build and flash the firmware to your keyboard.
7. Press `&ruen_macos 1` to switch `ruen` to macOS compatibility mode.

## Usage on Windows & Linux

> [!WARNING]  
> This module was only tested on macOS; Windows & Linux support is unverified.

1. Configure separate hotkeys for Russian and English layouts.  
2. In your `config/west.yml`, add these entries under `remotes` and `projects`:
   ```yaml
   manifest:
     remotes:
       - name: zmkfirmware
         url-base: https://github.com/zmkfirmware
       - name: jewtvet
         url-base: https://github.com/jewtvet
     projects:
       - name: zmk
         remote: zmkfirmware
         revision: main
         import: app/west.yml
       - name: zmk-ruen
         remote: jewtvet
         revision: main
     self:
       path: config
   ```
3. In your `config/<your-keyboard>.keymap`, under `behaviors`, add these entries, update `to_en` and `to_ru` to your own layout-switch hotkeys:
   ```yaml
   ruen_one_key: ruen_one_key {
     compatible = "zmk,behavior-ruen-one-key";
     #binding-cells = <2>;
     to_en = <0x8070025>;  # int code for your hotkey to switch to English (in this example: LG(N8))
     to_ru = <0x8070026>;  # int code for your hotkey to switch to Russian (in this example: LG(N9))
   };
   ```
4. Add your new keys into the same keymap file. 
5. Build and flash the firmware to your keyboard.

# Russian

Модуль `ruen` позволяет отправлять кейкоды независимо от выбранного языка и раскладки.

> [!WARNING]
> В настоящий момент модуль находится на ранней альфа‑версии, поэтому используйте его на свой страх и риск.

## Новые behaviors

- `&ruen_switch <0/1> <hotkey>` — переключает язык в системе и в клавиатуре согласно настройке:
  - `0`: внутренняя раскладка клавиатуры — русский
  - `1`: внутренняя раскладка клавиатуры — английский
  - `hotkey`: хоткей для смены раскладки в системе

- `&ruen_key <en_key> <ru_key>` — отправляет `en_key`, если текущий язык — английский, и `ru_key`, если текущий язык — русский:
  - `en_key`: клавиша для английской раскладки
  - `ru_key`: клавиша для русской раскладки

- `&ruen_one_key <0/1> <key>` — если текущий язык совпадает с выбранным при конфигурации, отправляет `key`; иначе выполняет: смена языка → отправка `key` → возвращение языка:
  - `0`: приоритет — русский язык
  - `1`: приоритет — английский язык
  - `key`: отправляемая клавиша

## Примеры использования

> [!NOTE]
> Примеры приведены для macOS с раскладками English и Russian.

- `&ruen_switch 0 LG(N9)` — внутренняя раскладка клавиатуры меняется на русский, отправляет `LGui+9` для переключения на русский в системе.
- `&ruen_switch 1 LG(N8)` — внутренняя раскладка клавиатуры меняется на английский, отправляет `LGui+8` для переключения на английский в системе.
- `&ruen_key LS(N5) LS(N4)` — при английской раскладке отправляет `LShift+5` (% в английской), при русской — `LShift+4` (% в русской). Всегда отправляет `%`.
- `&ruen_key Q W` — при английской раскладке отправляет `Q`, при русской — `W` (Ц). Позволяет объединить Colemak и "Диктор" раскладки в одном слое без изменения системных раскладок.
- `&ruen_one_key 1 LS(N3)` — при английской раскладке отправляет `LShift+3` (#); при русской — переключает на английский → отправляет `LShift+3` → возвращает русский. Всегда отправляет `#`.
- `&ruen_one_key 0 LS(N3)` — при русской раскладке отправляет `LShift+3` (№); при английской — переключает на русский → отправляет `LShift+3` → возвращает английский. Всегда отправляет `№`.

[Больше примеров в моей кеймапе](https://github.com/jewtvet-org/zmk-config-new/blob/main/config/cradio.keymap)

## Использование с macOS

1. Установите **MLSwitcher2** из Mac App Store и настройте отдельные хоткеи для русской и английской раскладок.
2. Добавьте следующие записи в `remotes` и `projects` в файле `config/west.yml`:
   ```yaml
   manifest:
     remotes:
       - name: zmkfirmware
         url-base: https://github.com/zmkfirmware
       - name: jewtvet
         url-base: https://github.com/jewtvet
     projects:
       - name: zmk
         remote: zmkfirmware
         revision: main
         import: app/west.yml
       - name: zmk-ruen
         remote: jewtvet
         revision: main
     self:
       path: config
   ```
3. В разделе `behaviors` в файле `config/<your-keyboard>.keymap` добавьте `ruen_one_key`:
   ```yaml
   ruen_one_key: ruen_one_key {
     compatible = "zmk,behavior-ruen-one-key";
     #binding-cells = <2>;
     to_en = <0x8070025>; # int код вашего хоткея для переключения на английский (в этом примере LG(N8))
     to_ru = <0x8070026>; # int код вашего хоткея для переключения на русский (в этом примере LG(N9))
   };
   ```
4. Добавьте новые клавиши в раскладку в том же файле.
5. Добавьте `&ruen_macos 1` на удаленное место в редко используемом слое. 
6. Соберите и загрузите прошивку на клавиатуру.
7. Нажмите `&ruen_macos 1`, чтобы перевести `ruen` в режим совместимости с macOS.

## Использование с Windows & Linux

> [!WARNING]
> Модуль проверялся только на macOS; работоспособность на Windows & Linux не гарантируется.

1. Настройте отдельные хоткеи для русской и английской раскладок.
2. Добавьте следующие записи в `remotes` и `projects` в файле `config/west.yml`:
   ```yaml
   manifest:
     remotes:
       - name: zmkfirmware
         url-base: https://github.com/zmkfirmware
       - name: jewtvet
         url-base: https://github.com/jewtvet
     projects:
       - name: zmk
         remote: zmkfirmware
         revision: main
         import: app/west.yml
       - name: zmk-ruen
         remote: jewtvet
         revision: main
     self:
       path: config
   ```
3. В разделе `behaviors` в файле `config/<your-keyboard>.keymap` добавьте `ruen_one_key`:
   ```yaml
   ruen_one_key: ruen_one_key {
     compatible = "zmk,behavior-ruen-one-key";
     #binding-cells = <2>;
     to_en = <0x8070025>; # int код вашего хоткея для переключения на английский (в этом примере LG(N8))
     to_ru = <0x8070026>; # int код вашего хоткея для переключения на русский (в этом примере LG(N9))
   };
   ```
4. Добавьте новые клавиши в раскладку в том же файле.
5. Соберите и загрузите прошивку на клавиатуру.