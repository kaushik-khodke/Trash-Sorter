import os

# Waste Categories and Single-Letter Code Mappings matching robotic_hand.ino EXACTLY:
# Plastic   -> 'P'  (robotic_hand.ino line 66)
# Paper     -> 'A'  (robotic_hand.ino line 82)
# Cardboard -> 'C'  (robotic_hand.ino line 98)
# Glass     -> 'G'  (robotic_hand.ino line 114)
# Metal     -> 'M'  (robotic_hand.ino line 130)

CATEGORIES = {
    "plastic": {
        "letter": "P",
        "display_name": "Plastic Waste",
        "prompts": [
            "clear plastic water bottle",
            "plastic container packaging",
            "plastic wrapper trash",
            "disposable plastic cup",
            "colored plastic bottle"
        ],
        "color": (50, 180, 255) # BGR
    },
    "paper": {
        "letter": "A",
        "display_name": "Paper Waste",
        "prompts": [
            "sheets of paper waste",
            "wrinkled paper sheet",
            "printed document paper",
            "notebook paper trash",
            "newspaper paper waste"
        ],
        "color": (255, 200, 100)
    },
    "cardboard": {
        "letter": "C",
        "display_name": "Cardboard Waste",
        "prompts": [
            "brown corrugated cardboard box",
            "cardboard shipping box packaging",
            "cardboard paperboard box",
            "folded cardboard piece"
        ],
        "color": (40, 120, 200)
    },
    "glass": {
        "letter": "G",
        "display_name": "Glass Waste",
        "prompts": [
            "transparent glass bottle container",
            "green glass bottle jar",
            "brown glass bottle waste",
            "glass jar trash"
        ],
        "color": (0, 220, 120)
    },
    "metal": {
        "letter": "M",
        "display_name": "Metal Waste",
        "prompts": [
            "aluminum metal beverage soda can",
            "crushed metal tin can",
            "metal food container trash",
            "aluminum metal foil",
            "metallic object"
        ],
        "color": (180, 180, 180)
    }
}

# Model & Decision Parameters
MODEL_NAME = "openai/clip-vit-base-patch32"
FALLBACK_MODEL_NAME = "openai/clip-vit-base-patch16"
DEVICE = "cpu"  # 'cuda' if available, automatically detected

# Temporal Stability & State Machine Settings
THINKING_DURATION = 2.5      # Seconds spent sampling frames before committing to an answer
CLEAR_ROI_RESET_TIME = 1.0   # Seconds of empty background required to reset state after item is lifted
CONFIDENCE_THRESHOLD = 0.45   # Minimum prompt probability to trigger object presence

# Camera Settings
DEFAULT_CAMERA_INDEX = 0
FRAME_WIDTH = 1280
FRAME_HEIGHT = 720

# IIoT Serial Communication Settings matching robotic_hand.ino (19200 baud)
SERIAL_PORT = "COM3"
BAUD_RATE = 19200            # Matches Serial.begin(19200) in robotic_hand.ino!
MOCK_SERIAL = True            # Set False when physical Arduino/ESP32 is connected
