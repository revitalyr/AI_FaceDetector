// Image Provider Module - handles image loading from file system and provides to web
class ImageProvider {
    constructor() {
        this.imageCache = new Map();
        this.currentImageIndex = 0;
        this.availableImages = [];
        this.loadImageList();
    }

    // Load list of available images from file system
    async loadImageList() {
        try {
            const response = await fetch('/api/images');
            if (!response.ok) {
                throw new Error('Failed to load image list');
            }
            
            const images = await response.json();
            this.availableImages = images;
            
            // Find 300_1.jpg and set as current
            const targetIndex = this.availableImages.findIndex(img => img.name === '300_1.jpg');
            this.currentImageIndex = targetIndex >= 0 ? targetIndex : 0;
            
            console.log(`ImageProvider: Loaded ${this.availableImages.length} images`);
            console.log(`ImageProvider: Current image: ${this.getCurrentImageName()}`);
            
            // Notify listeners that images are loaded
            this.notifyImagesLoaded();
            
        } catch (error) {
            console.error('ImageProvider: Failed to load image list:', error);
            this.availableImages = [];
        }
    }

    // Get current image data for analysis
    async getCurrentImageData() {
        if (this.availableImages.length === 0) {
            throw new Error('No images available');
        }

        const currentImage = this.availableImages[this.currentImageIndex];
        
        // Check cache first
        if (this.imageCache.has(currentImage.url)) {
            return this.imageCache.get(currentImage.url);
        }

        // Load image from server
        try {
            const response = await fetch(currentImage.url);
            if (!response.ok) {
                throw new Error(`Failed to load image: ${currentImage.name}`);
            }

            const blob = await response.blob();
            const imageData = await this.blobToImageData(blob);
            
            // Cache the result
            this.imageCache.set(currentImage.url, imageData);
            
            return imageData;
            
        } catch (error) {
            console.error('ImageProvider: Failed to load current image:', error);
            throw error;
        }
    }

    // Convert blob to image data
    async blobToImageData(blob) {
        return new Promise((resolve, reject) => {
            const img = new Image();
            
            img.onload = () => {
                const canvas = document.createElement('canvas');
                canvas.width = img.width;
                canvas.height = img.height;
                const ctx = canvas.getContext('2d');
                
                // Draw image to canvas
                ctx.drawImage(img, 0, 0);
                
                // Get image data
                const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
                
                resolve({
                    imageData: imageData,
                    canvas: canvas,
                    width: img.width,
                    height: img.height,
                    name: this.getCurrentImageName(),
                    url: this.getCurrentImageUrl()
                });
            };
            
            img.onerror = () => {
                reject(new Error('Failed to convert blob to image'));
            };
            
            img.src = URL.createObjectURL(blob);
        });
    }

    // Get current image name
    getCurrentImageName() {
        if (this.availableImages.length === 0) return null;
        return this.availableImages[this.currentImageIndex].name;
    }

    // Get current image URL
    getCurrentImageUrl() {
        if (this.availableImages.length === 0) return null;
        return this.availableImages[this.currentImageIndex].url;
    }

    // Get current image index
    getCurrentIndex() {
        return this.currentImageIndex;
    }

    // Get total number of images
    getTotalImages() {
        return this.availableImages.length;
    }

    // Navigate to next image
    nextImage() {
        if (this.availableImages.length === 0) return false;
        
        this.currentImageIndex = (this.currentImageIndex + 1) % this.availableImages.length;
        console.log(`ImageProvider: Next image: ${this.getCurrentImageName()}`);
        
        // Notify listeners
        this.notifyImageChanged();
        
        return true;
    }

    // Navigate to previous image
    previousImage() {
        if (this.availableImages.length === 0) return false;
        
        this.currentImageIndex = (this.currentImageIndex - 1 + this.availableImages.length) % this.availableImages.length;
        console.log(`ImageProvider: Previous image: ${this.getCurrentImageName()}`);
        
        // Notify listeners
        this.notifyImageChanged();
        
        return true;
    }

    // Set image by index
    setImageByIndex(index) {
        if (index >= 0 && index < this.availableImages.length) {
            this.currentImageIndex = index;
            console.log(`ImageProvider: Set image: ${this.getCurrentImageName()}`);
            
            // Notify listeners
            this.notifyImageChanged();
            
            return true;
        }
        return false;
    }

    // Check if images are loaded
    isReady() {
        return this.availableImages.length > 0;
    }

    // Get all available images info
    getAvailableImages() {
        return this.availableImages.map(img => ({
            name: img.name,
            url: img.url
        }));
    }

    // Clear cache
    clearCache() {
        this.imageCache.clear();
        console.log('ImageProvider: Cache cleared');
    }

    // Event listeners
    addEventListener(event, callback) {
        if (!this.listeners) {
            this.listeners = {};
        }
        if (!this.listeners[event]) {
            this.listeners[event] = [];
        }
        this.listeners[event].push(callback);
    }

    removeEventListener(event, callback) {
        if (this.listeners && this.listeners[event]) {
            const index = this.listeners[event].indexOf(callback);
            if (index > -1) {
                this.listeners[event].splice(index, 1);
            }
        }
    }

    // Notify listeners
    notifyImagesLoaded() {
        if (this.listeners && this.listeners['imagesLoaded']) {
            this.listeners['imagesLoaded'].forEach(callback => {
                callback(this.getAvailableImages());
            });
        }
    }

    notifyImageChanged() {
        if (this.listeners && this.listeners['imageChanged']) {
            this.listeners['imageChanged'].forEach(callback => {
                callback({
                    name: this.getCurrentImageName(),
                    url: this.getCurrentImageUrl(),
                    index: this.getCurrentIndex(),
                    total: this.getTotalImages()
                });
            });
        }
    }

    // Get image for web display (returns canvas)
    async getImageForWeb() {
        const imageData = await this.getCurrentImageData();
        return imageData.canvas;
    }

    // Get image data for analysis
    async getImageForAnalysis() {
        const imageData = await this.getCurrentImageData();
        return imageData.imageData;
    }

    // Get image metadata
    getCurrentImageMetadata() {
        if (this.availableImages.length === 0) return null;
        
        const currentImage = this.availableImages[this.currentImageIndex];
        return {
            name: currentImage.name,
            url: currentImage.url,
            index: this.currentImageIndex,
            total: this.availableImages.length,
            path: currentImage.path
        };
    }
}

// Export for use
window.ImageProvider = ImageProvider;
