// Image Processor - Handles exposure, contrast, gamma, saturation
class ImageProcessor {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.originalImageData = null;
        this.currentImageData = null;
        
        // Processing parameters
        this.exposure = 0;
        this.contrast = 1.0;
        this.gamma = 1.0;
        this.saturation = 1.0;
    }

    // Set original image
    setOriginalImage(imageData) {
        this.originalImageData = imageData;
        this.currentImageData = this.ctx.createImageData(imageData);
        this.applyAllAdjustments();
    }

    // Apply all adjustments
    applyAllAdjustments() {
        if (!this.originalImageData) return;
        
        const processedData = this.processImageData(this.originalImageData);
        this.currentImageData.data.set(processedData);
        this.ctx.putImageData(this.currentImageData, 0, 0);
        
        // Update histogram
        if (window.histogram) {
            window.histogram.updateHistogram(processedData);
        }
    }

    // Process image data with all adjustments
    processImageData(imageData) {
        const data = new Uint8ClampedArray(imageData.data);
        const length = data.length;
        
        for (let i = 0; i < length; i += 4) {
            let r = data[i];
            let g = data[i + 1];
            let b = data[i + 2];
            
            // Apply exposure
            r = this.applyExposure(r, this.exposure);
            g = this.applyExposure(g, this.exposure);
            b = this.applyExposure(b, this.exposure);
            
            // Apply contrast
            r = this.applyContrast(r, this.contrast);
            g = this.applyContrast(g, this.contrast);
            b = this.applyContrast(b, this.contrast);
            
            // Apply gamma correction
            r = this.applyGamma(r, this.gamma);
            g = this.applyGamma(g, this.gamma);
            b = this.applyGamma(b, this.gamma);
            
            // Apply saturation
            const rgb = this.applySaturation(r, g, b, this.saturation);
            data[i] = rgb.r;
            data[i + 1] = rgb.g;
            data[i + 2] = rgb.b;
        }
        
        return data;
    }

    // Exposure adjustment (-3 to +3)
    applyExposure(value, exposure) {
        return Math.max(0, Math.min(255, value * Math.pow(2, exposure)));
    }

    // Contrast adjustment (0.7 to 1.5)
    applyContrast(value, contrast) {
        return Math.max(0, Math.min(255, 128 + (value - 128) * contrast));
    }

    // Gamma correction (0.5 to 2.0)
    applyGamma(value, gamma) {
        return Math.max(0, Math.min(255, 255 * Math.pow(value / 255, 1 / gamma)));
    }

    // Saturation adjustment (0 to 2.0)
    applySaturation(r, g, b, saturation) {
        const gray = 0.299 * r + 0.587 * g + 0.114 * b;
        return {
            r: Math.max(0, Math.min(255, gray + (r - gray) * saturation)),
            g: Math.max(0, Math.min(255, gray + (g - gray) * saturation)),
            b: Math.max(0, Math.min(255, gray + (b - gray) * saturation))
        };
    }

    // Set individual parameters
    setExposure(value) {
        this.exposure = parseFloat(value);
        this.applyAllAdjustments();
    }

    setContrast(value) {
        this.contrast = parseFloat(value);
        this.applyAllAdjustments();
    }

    setGamma(value) {
        this.gamma = parseFloat(value);
        this.applyAllAdjustments();
    }

    setSaturation(value) {
        this.saturation = parseFloat(value);
        this.applyAllAdjustments();
    }

    // Reset to original
    reset() {
        this.exposure = 0;
        this.contrast = 1.0;
        this.gamma = 1.0;
        this.saturation = 1.0;
        this.applyAllAdjustments();
    }

    // Get current processed image data
    getCurrentImageData() {
        return this.currentImageData;
    }
}

// Export for use
window.ImageProcessor = ImageProcessor;
