// Simple Image Provider - based on working code from simple-real-test.html
class SimpleImageProvider {
    constructor() {
        this.images = [];
        this.currentIndex = 0;
        this.imageCache = new Map();
        this.loadImages();
    }

    // Load images from server API
    async loadImages() {
        try {
            const response = await fetch('/api/images');
            if (!response.ok) {
                throw new Error(`Server error: ${response.status}`);
            }
            
            this.images = await response.json();
            
            // Find 300_1.jpg and set as current
            const targetIndex = this.images.findIndex(img => img.name === '300_1.jpg');
            this.currentIndex = targetIndex >= 0 ? targetIndex : 0;
            
            console.log(`SimpleImageProvider: Loaded ${this.images.length} real images`);
            console.log(`SimpleImageProvider: Current image: ${this.getCurrentImageName()}`);
            
            // Notify listeners
            this.notifyImagesLoaded();
            
        } catch (error) {
            console.error('SimpleImageProvider: Failed to load images:', error);
            this.images = [];
        }
    }

    // Get current image data - using working code from simple-real-test.html
    async getCurrentImageData() {
        if (this.images.length === 0) {
            throw new Error('No images available');
        }

        const currentImage = this.images[this.currentIndex];
        
        // Check cache first
        if (this.imageCache.has(currentImage.url)) {
            return this.imageCache.get(currentImage.url);
        }

        try {
            console.log(`SimpleImageProvider: Loading real image: ${currentImage.name}`);
            
            // Use working code from simple-real-test.html
            const imgResponse = await fetch(currentImage.url);
            if (!imgResponse.ok) {
                throw new Error(`Failed to load ${currentImage.name}: ${imgResponse.status}`);
            }
            
            const blob = await imgResponse.blob();
            const img = new Image();
            
            await new Promise((resolve, reject) => {
                img.onload = resolve;
                img.onerror = reject;
                img.src = URL.createObjectURL(blob);
            });
            
            // Create canvas and get image data
            const canvas = document.createElement('canvas');
            canvas.width = img.width;
            canvas.height = img.height;
            const ctx = canvas.getContext('2d');
            
            // Draw image to canvas
            ctx.drawImage(img, 0, 0);
            
            // Get image data
            const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
            
            const result = {
                imageData: imageData,
                canvas: canvas,
                width: img.width,
                height: img.height,
                name: currentImage.name,
                url: currentImage.url
            };
            
            // Cache the result
            this.imageCache.set(currentImage.url, result);
            
            console.log(`✅ SimpleImageProvider: Successfully loaded REAL image: ${currentImage.name} (${img.width}×${img.height})`);
            
            return result;
            
        } catch (error) {
            console.error(`SimpleImageProvider: Failed to load ${currentImage.name}:`, error);
            throw error;
        }
    }

    // Get current image name
    getCurrentImageName() {
        if (this.images.length === 0) return null;
        return this.images[this.currentIndex].name;
    }

    // Get current image URL
    getCurrentImageUrl() {
        if (this.images.length === 0) return null;
        return this.images[this.currentIndex].url;
    }

    // Get current index
    getCurrentIndex() {
        return this.currentIndex;
    }

    // Get total images
    getTotalImages() {
        return this.images.length;
    }

    // Navigate to next image
    nextImage() {
        if (this.images.length === 0) return false;
        
        this.currentIndex = (this.currentIndex + 1) % this.images.length;
        console.log(`SimpleImageProvider: Next image: ${this.getCurrentImageName()}`);
        
        this.notifyImageChanged();
        return true;
    }

    // Navigate to previous image
    previousImage() {
        if (this.images.length === 0) return false;
        
        this.currentIndex = (this.currentIndex - 1 + this.images.length) % this.images.length;
        console.log(`SimpleImageProvider: Previous image: ${this.getCurrentImageName()}`);
        
        this.notifyImageChanged();
        return true;
    }

    // Check if ready
    isReady() {
        return this.images.length > 0;
    }

    // Get available images
    getAvailableImages() {
        return this.images.map(img => ({
            name: img.name,
            url: img.url
        }));
    }

    // Clear cache
    clearCache() {
        this.imageCache.clear();
        console.log('SimpleImageProvider: Cache cleared');
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
}

// Export for use
window.SimpleImageProvider = SimpleImageProvider;
