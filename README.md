# IIoT Mechanical Arm Trash Sorter (`robotic_hand.ino` Integration)

An AI vision system designed for IIoT garbage sorting using a mechanical arm (`robotic_hand.ino`) and a laptop camera. 
The Python application matches the exact serial commands and baud rate defined in `robotic_hand.ino`.

---

## Mappings (robotic_hand.ino Protocol)

| Waste Category | Transmitted Single-Letter Code | Arduino Handler Function | Mechanical Arm Action |
| :--- | :---: | :--- | :--- |
| **Plastic Waste** | **`P`** | `throwPlastic()` *(Line 66)* | Picks object & throws into **Plastic Bin** |
| **Paper Waste** | **`A`** | `throwPaper()` *(Line 82)* | Picks object & throws into **Paper Bin** |
| **Cardboard Waste** | **`C`** | `throwCardboard()` *(Line 98)* | Picks object & throws into **Cardboard Bin** |
| **Glass Waste** | **`G`** | `throwGlass()` *(Line 114)* | Picks object & throws into **Glass Bin** |
| **Metal Waste** | **`M`** | `throwMetal()` *(Line 130)* | Picks object & throws into **Metal Bin** |

---

## Operating Protocol & State Protection

1. **`19200 Baud Rate`**: Configured to match `Serial.begin(19200)` in `robotic_hand.ino`.
2. **`2.5s Thinking Phase`**: When an item enters the target box, the vision model samples frames over 2.5s for 100% consensus classification.
3. **`Single Command Dispatch`**: Transmits the code (`P`, `A`, `C`, `G`, `M`) **EXACTLY ONCE**.
4. **`Model Pause & Busy Lock State`**:
   - As soon as the command is sent, the system enters **`ARM OPERATING (MODEL PAUSED)`**.
   - **Model guessing is completely PAUSED** while the robotic arm is picking up and throwing the item.
   - **No new commands** can be sent mid-process to prevent confusing the robotic arm.
5. **`Arduino Handshake ("Done" Acknowledgement)`**:
   - Upon completing the throw routine, `robotic_hand.ino` outputs `Serial.println("Done")`.
   - The Python application listens for `"Done"`, unlocks the busy state, and only THEN resumes model guessing for the next item!
6. **`Reload / Reset Button [R]`**:
   - Pressing **`R`** on your keyboard instantly resets the state machine back to start phase (`WAITING FOR ITEM`) if anything ever gets locked or needs a manual reset.

---

## How to Run

1. **Launch Sorter Application**:
   ```bash
   python main.py
   ```

2. **Connect Physical Robotic Arm**:
   Set `MOCK_SERIAL = False` in `config.py` and specify your Arduino COM port (e.g. `COM3`).


### OPEN SOURCE MODEL USED == openai/clip-vit-large-patch14