// Image Loader - Handles real file loading and processing
class ImageLoader {
    constructor() {
        this.supportedFormats = ['jpg', 'jpeg', 'png', 'gif', 'bmp', 'webp'];
        this.maxFileSize = 10 * 1024 * 1024; // 10MB
    }

    // Load image from file
    loadFromFile(file) {
        return new Promise((resolve, reject) => {
            // Validate file
            if (!this.validateFile(file)) {
                reject(new Error('Invalid file format or size'));
                return;
            }

            const reader = new FileReader();
            reader.onload = (e) => {
                const img = new Image();
                img.onload = () => {
                    // Create canvas and process image
                    const canvas = document.createElement('canvas');
                    canvas.width = img.width;
                    canvas.height = img.height;
                    const ctx = canvas.getContext('2d');
                    
                    // Draw image to canvas
                    ctx.drawImage(img, 0, 0);
                    
                    // Get image data
                    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
                    
                    resolve({
                        canvas: canvas,
                        imageData: imageData,
                        width: img.width,
                        height: img.height,
                        fileName: file.name
                    });
                };
                
                img.onerror = () => reject(new Error('Failed to load image'));
                img.src = e.target.result;
            };
            
            reader.onerror = () => reject(new Error('Failed to read file'));
            reader.readAsDataURL(file);
        });
    }

    // Validate file format and size
    validateFile(file) {
        // Check file extension
        const extension = file.name.split('.').pop().toLowerCase();
        if (!this.supportedFormats.includes(extension)) {
            return false;
        }

        // Check file size
        if (file.size > this.maxFileSize) {
            return false;
        }

        return true;
    }

    // Get supported formats
    getSupportedFormats() {
        return this.supportedFormats;
    }

    // Format file size for display
    formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }
}

// Export for use
window.ImageLoader = ImageLoader;
