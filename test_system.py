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
        self.assertIn("is_object_present", analysis)

    def test_object_presence_detector_wall_vs_object(self):
        """Verify ObjectPresenceDetector differentiates plain/wall background from foreground object."""
        from model import ObjectPresenceDetector
        detector = ObjectPresenceDetector()

        # Uniform plain wall image
        plain_wall = np.ones((300, 300, 3), dtype=np.uint8) * 210
        is_present_wall, score_wall = detector.detect_presence(plain_wall)
        self.assertFalse(is_present_wall, f"Plain wall misidentified as object! (score={score_wall})")

        # Simulated object with high edge density and contrast
        object_img = np.ones((300, 300, 3), dtype=np.uint8) * 210
        import cv2
        cv2.rectangle(object_img, (50, 50), (250, 250), (10, 10, 10), -1)
        cv2.circle(object_img, (150, 150), 40, (255, 255, 255), -1)
        is_present_obj, score_obj = detector.detect_presence(object_img)
        self.assertTrue(is_present_obj, f"Object failed presence detection! (score={score_obj})")

    def test_classifier_wall_filtering(self):
        """Verify classifier sets is_valid=False for wall / plain backgrounds."""
        classifier = OptimizedWasteClassifier()
        wall_img = np.ones((300, 300, 3), dtype=np.uint8) * 180
        analysis = classifier.analyze_frame(wall_img)
        self.assertFalse(analysis["is_valid"], "Wall background incorrectly classified as valid trash object!")
        self.assertFalse(analysis["is_object_present"], "Wall background failed presence detection check!")

if __name__ == "__main__":
    unittest.main()
