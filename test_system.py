import unittest
import numpy as np
import config
from iiot_communicator import IIoTCommunicator
from model import OptimizedWasteClassifier

class TestTrashSorterSystem(unittest.TestCase):
    def setUp(self):
        self.iiot = IIoTCommunicator(mock=True)

    def test_single_letter_mappings_robotic_hand_ino(self):
        """Verify exact single letter codes specified in robotic_hand.ino."""
        self.assertEqual(config.CATEGORIES["plastic"]["letter"], "P")   # Line 66
        self.assertEqual(config.CATEGORIES["paper"]["letter"], "A")     # Line 82
        self.assertEqual(config.CATEGORIES["cardboard"]["letter"], "C") # Line 98
        self.assertEqual(config.CATEGORIES["glass"]["letter"], "G")     # Line 114
        self.assertEqual(config.CATEGORIES["metal"]["letter"], "M")     # Line 130
        self.assertEqual(config.BAUD_RATE, 19200)

    def test_iiot_transmission(self):
        """Test sending valid single-letter codes to robotic arm."""
        for code in ["P", "A", "C", "G", "M"]:
            result = self.iiot.send_code(code)
            self.assertTrue(result, f"Failed to send valid single-letter code '{code}'")

        # Invalid letters ignored
        self.assertFalse(self.iiot.send_code("U"))
        self.assertFalse(self.iiot.send_code("X"))
        self.assertFalse(self.iiot.send_code(""))

    def test_optimized_model_initialization(self):
        """Verify OptimizedWasteClassifier initialization."""
        classifier = OptimizedWasteClassifier()
        dummy_img = np.ones((300, 300, 3), dtype=np.uint8) * 128
        analysis = classifier.analyze_frame(dummy_img)
        self.assertIsNotNone(analysis)
        self.assertIn("probabilities", analysis)
        self.assertIn("bg_prob", analysis)

if __name__ == "__main__":
    unittest.main()
