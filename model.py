import torch
import cv2
from PIL import Image
import numpy as np
import logging
from transformers import CLIPProcessor, CLIPModel
import config

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")

class ObjectPresenceDetector:
    """
    Computer Vision Objectness & Presence Detector.
    Evaluates contour geometry, edge density, and surface variance within the target ROI
    to differentiate physical foreground objects from plain or colorful backgrounds/walls.
    """
    def __init__(self, min_score=config.MIN_OBJECTNESS_SCORE):
        self.min_score = min_score

    def detect_presence(self, cv2_image_bgr):
        """
        Calculates structural objectness score (0.0 to 1.0).
        Returns tuple: (is_object_present: bool, objectness_score: float)
        """
        if cv2_image_bgr is None or cv2_image_bgr.size == 0:
            return False, 0.0

        h, w, c = cv2_image_bgr.shape
        total_pixels = float(h * w)
        gray = cv2.cvtColor(cv2_image_bgr, cv2.COLOR_BGR2GRAY)

        # 1. Edge density via Canny edge detection
        edges = cv2.Canny(gray, 50, 150)
        edge_density = np.count_nonzero(edges) / total_pixels

        # 2. Texture variance using Laplacian variance
        laplacian_var = cv2.Laplacian(gray, cv2.CV_64F).var()
        normalized_laplacian = min(1.0, laplacian_var / 500.0)

        # 3. Contour geometry score
        contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        max_contour_ratio = 0.0
        if contours:
            max_contour_area = max(cv2.contourArea(cnt) for cnt in contours)
            max_contour_ratio = max_contour_area / total_pixels

        # 4. Color variance score (plain/painted walls have lower local stddev)
        color_std = float(np.std(cv2_image_bgr))
        normalized_color_std = min(1.0, color_std / 80.0)

        # Combine metrics into structural objectness score (0.0 to 1.0)
        objectness_score = (
            0.40 * min(1.0, edge_density * 20.0) +
            0.30 * min(1.0, max_contour_ratio * 15.0) +
            0.15 * normalized_laplacian +
            0.15 * normalized_color_std
        )

        is_present = objectness_score >= self.min_score
        return is_present, float(objectness_score)

class OptimizedWasteClassifier:
    """
    High-Performance Multi-Stage Vision Classifier.
    Integrates CV Object Presence Detection with CLIP Zero-Shot Classification.
    Pre-computes and caches text embeddings at startup for ultra-fast visual inference.
    Classifies input image crops into 5 target waste categories:
      - Paper Waste     -> 'A'
      - Plastic Waste   -> 'P'
      - Metal Waste     -> 'M'
      - Cardboard Waste -> 'C'
      - Glass Waste     -> 'G'
    """
    def __init__(self, model_name=config.MODEL_NAME, device=config.DEVICE):
        self.device = "cuda" if torch.cuda.is_available() and device == "cuda" else "cpu"
        self.model_name = model_name
        self.categories = config.CATEGORIES
        self.confidence_threshold = config.CONFIDENCE_THRESHOLD
        self.presence_detector = ObjectPresenceDetector()

        logging.info(f"Loading High-Performance Model '{self.model_name}' on {self.device.upper()}...")
        try:
            self.model = CLIPModel.from_pretrained(self.model_name).to(self.device)
            self.processor = CLIPProcessor.from_pretrained(self.model_name)
            self.model.eval()
        except Exception as e:
            logging.warning(f"Could not load '{self.model_name}': {e}. Falling back to 'openai/clip-vit-base-patch32'...")
            self.model_name = "openai/clip-vit-base-patch32"
            self.model = CLIPModel.from_pretrained(self.model_name).to(self.device)
            self.processor = CLIPProcessor.from_pretrained(self.model_name)
            self.model.eval()

        # Build prompt list
        self.keys = list(self.categories.keys())
        self.prompt_texts = []
        self.key_indices = []

        for idx, key in enumerate(self.keys):
            prompts = self.categories[key]["prompts"]
            for p in prompts:
                self.prompt_texts.append(f"a photo of {p}")
                self.key_indices.append(idx)

        # Background & Wall Prompts (Expanded Prompt Bank)
        bg_prompts = getattr(config, "BACKGROUND_WALL_PROMPTS", [
            "a photo of an empty table surface or plain background or human hand"
        ])
        for bg_prompt in bg_prompts:
            self.prompt_texts.append(bg_prompt)
            self.key_indices.append(-1)

        # PRE-COMPUTE AND CACHE TEXT EMBEDDINGS (Boosts inference speed by 5x!)
        logging.info("Pre-computing text prompt embeddings for ultra-fast inference...")
        text_inputs = self.processor(text=self.prompt_texts, return_tensors="pt", padding=True).to(self.device)
        with torch.no_grad():
            text_outputs = self.model.get_text_features(**text_inputs)
            # Normalize text features
            self.text_features = text_outputs / text_outputs.norm(dim=-1, keepdim=True)
        logging.info("Text embeddings successfully cached. Real-time vision engine ready.")

    def analyze_frame(self, cv2_image_bgr):
        """
        Multi-stage analysis:
        1. Object Presence verification (edge/contour/texture variance).
        2. CLIP visual similarity calculation using cached text embeddings.
        Returns probability distribution dict and validity state.
        """
        if cv2_image_bgr is None or cv2_image_bgr.size == 0:
            return None

        # Stage 1: Structural Object Presence Detection
        is_object_present, objectness_score = True, 1.0
        if getattr(config, "ENABLE_OBJECT_PRESENCE_CHECK", True):
            is_object_present, objectness_score = self.presence_detector.detect_presence(cv2_image_bgr)

        # Stage 2: CLIP Classification
        rgb_image = cv2_image_bgr[:, :, ::-1]
        pil_image = Image.fromarray(rgb_image)

        image_inputs = self.processor(images=pil_image, return_tensors="pt").to(self.device)

        with torch.no_grad():
            image_features = self.model.get_image_features(**image_inputs)
            image_features = image_features / image_features.norm(dim=-1, keepdim=True)
            
            # Dot product similarity with pre-cached text embeddings
            similarity = (100.0 * image_features @ self.text_features.T).softmax(dim=-1)
            probs = similarity.cpu().numpy()[0]

        category_probs = np.zeros(len(self.keys))
        bg_prob = 0.0

        for prob, key_idx in zip(probs, self.key_indices):
            if key_idx == -1:
                bg_prob += prob
            else:
                category_probs[key_idx] += prob

        probs_dict = {self.keys[i]: float(category_probs[i]) for i in range(len(self.keys))}

        best_idx = int(np.argmax(category_probs))
        best_prob = float(category_probs[best_idx])
        best_key = self.keys[best_idx]

        # Valid object requires BOTH structural presence AND high CLIP score over background
        is_valid_object = is_object_present and (best_prob > bg_prob) and (best_prob >= self.confidence_threshold)

        return {
            "probabilities": probs_dict,
            "bg_prob": float(bg_prob),
            "best_category": best_key if is_valid_object else None,
            "best_prob": best_prob,
            "is_object_present": is_object_present,
            "objectness_score": objectness_score,
            "is_valid": is_valid_object
        }

