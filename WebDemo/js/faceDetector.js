// Face Detection - Improved fake face detection for demo purposes
class FaceDetector {
    constructor() {
        this.faces = [];
        this.isDetecting = false;
    }

    // Fake face detection - generates realistic face positions
    detectFaces(imageData) {
        if (this.isDetecting) return;
        
        this.isDetecting = true;
        
        // Update UI status to detecting
        if (window.ui) {
            window.ui.updateDetectionStatus('detecting', 'Detecting faces...');
        }
        
        // Simulate detection delay
        setTimeout(() => {
            this.faces = this.generateRealisticFaces(imageData);
            this.isDetecting = false;
            
            // Trigger face detection event
            if (window.app) {
                window.app.onFacesDetected(this.faces);
            }
        }, 300);
    }

    // Generate realistic face positions based on image content
    generateRealisticFaces(imageData) {
        const faces = [];
        
        if (!imageData) return faces;
        
        const width = imageData.width;
        const height = imageData.height;
        const data = imageData.data;
        
        // Simple face detection simulation
        // Look for skin-colored regions in the center area
        const centerX = width / 2;
        const centerY = height / 2;
        
        // Check if this looks like a portrait (has skin tones in center)
        let hasSkinTones = false;
        let skinRegionCount = 0;
        
        for (let y = Math.floor(height * 0.3); y < Math.floor(height * 0.7); y += 5) {
            for (let x = Math.floor(width * 0.3); x < Math.floor(width * 0.7); x += 5) {
                const idx = (y * width + x) * 4;
                const r = data[idx];
                const g = data[idx + 1];
                const b = data[idx + 2];
                
                // Simple skin tone detection
                if (this.isSkinTone(r, g, b)) {
                    skinRegionCount++;
                    if (skinRegionCount > 10) {
                        hasSkinTones = true;
                        break;
                    }
                }
            }
            if (hasSkinTones) break;
        }
        
        if (hasSkinTones) {
            // Generate face position based on image analysis
            const faceWidth = Math.min(width * 0.25, 80);
            const faceHeight = Math.min(height * 0.35, 100);
            
            faces.push({
                x: centerX - faceWidth/2 + (Math.random() - 0.5) * 20,
                y: centerY - faceHeight/2 + (Math.random() - 0.5) * 20,
                width: faceWidth + (Math.random() - 0.5) * 10,
                height: faceHeight + (Math.random() - 0.5) * 10,
                confidence: 0.75 + Math.random() * 0.2
            });
            
            // Sometimes detect multiple faces
            if (Math.random() > 0.7 && width > 200) {
                const secondFaceX = centerX < width/2 ? centerX + faceWidth : centerX - faceWidth * 1.5;
                faces.push({
                    x: secondFaceX + (Math.random() - 0.5) * 20,
                    y: centerY - faceHeight/2 + (Math.random() - 0.5) * 20,
                    width: faceWidth * 0.8,
                    height: faceHeight * 0.8,
                    confidence: 0.65 + Math.random() * 0.15
                });
            }
        } else {
            // For non-portrait images, occasionally detect "false positives"
            if (Math.random() > 0.8) {
                faces.push({
                    x: Math.random() * width * 0.8 + width * 0.1,
                    y: Math.random() * height * 0.8 + height * 0.1,
                    width: 40 + Math.random() * 30,
                    height: 50 + Math.random() * 30,
                    confidence: 0.4 + Math.random() * 0.2
                });
            }
        }
        
        return faces;
    }

    // Simple skin tone detection
    isSkinTone(r, g, b) {
        // Basic skin tone ranges
        return (
            r > 95 && g > 40 && b > 20 &&
            r > g && r > b &&
            Math.abs(r - g) < 30 &&
            r - b > 15
        );
    }

    // Get detected faces
    getFaces() {
        return this.faces;
    }

    // Clear faces
    clearFaces() {
        this.faces = [];
    }

    // Get combined bounds of all faces
    getFaceBounds() {
        if (this.faces.length === 0) return null;
        
        let minX = Infinity, minY = Infinity;
        let maxX = -Infinity, maxY = -Infinity;
        
        this.faces.forEach(face => {
            minX = Math.min(minX, face.x);
            minY = Math.min(minY, face.y);
            maxX = Math.max(maxX, face.x + face.width);
            maxY = Math.max(maxY, face.y + face.height);
        });
        
        return {
            x: minX,
            y: minY,
            width: maxX - minX,
            height: maxY - minY
        };
    }
}

// Export for use
window.FaceDetector = FaceDetector;
