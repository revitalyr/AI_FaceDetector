// Main Application Controller
class App {
    constructor() {
        this.canvas = document.getElementById('mainCanvas');
        this.rightCanvas = document.getElementById('rightCanvas');
        this.imageProcessor = null;
        this.histogram = null;
        this.faceDetector = null;
        this.splitScreen = null;
        this.ui = null;
        this.imageGenerator = null;
        this.imageLoader = null;
        this.imageProvider = null;
        this.currentImage = null;
        this.mousePos = { x: 0, y: 0 };
        
        this.init();
    }

    // Initialize application
    init() {
        // Initialize components
        this.imageProcessor = new ImageProcessor(this.canvas);
        this.histogram = new Histogram(document.getElementById('histogramCanvas'));
        this.faceDetector = new FaceDetector();
        this.splitScreen = new SplitScreen(this.canvas, this.rightCanvas);
        this.ui = new UIController();
        this.imageGenerator = new ImageGenerator();
        this.imageLoader = new ImageLoader();
        this.imageProvider = new SimpleImageProvider(); // Use working SimpleImageProvider
        
        // Set up canvas mouse tracking
        this.initCanvasEvents();
        
        // Set up file input
        this.initFileInput();
        
        // Set up drag and drop
        this.initDragAndDrop();
        
        // Set up demo navigation
        this.initDemoNavigation();
        
        // Set up image provider events
        this.initImageProviderEvents();
        
        // Wait for images to load
        this.waitForImagesAndLoad();
        
        // Make components globally available
        window.app = this;
        window.histogram = this.histogram;
        window.imageProcessor = this.imageProcessor;
        window.faceDetector = this.faceDetector;
        window.splitScreen = this.splitScreen;
        window.ui = this.ui;
        window.imageProvider = this.imageProvider;
        
        console.log('Glance Demo initialized with SimpleImageProvider (working code from simple-real-test.html)');
    }

    // Initialize image provider events
    initImageProviderEvents() {
        // Listen for image changes
        this.imageProvider.addEventListener('imageChanged', (imageInfo) => {
            console.log('Image changed:', imageInfo);
            this.loadCurrentImage();
        });

        // Listen for images loaded
        this.imageProvider.addEventListener('imagesLoaded', (images) => {
            console.log('Images loaded:', images.length);
            this.updateImageCounter();
        });
    }

    // Wait for images to be loaded from ImageProvider
    async waitForImagesAndLoad() {
        console.log('waitForImagesAndLoad: Starting...');
        console.log('ImageProvider ready:', this.imageProvider.isReady());
        console.log('Available images:', this.imageProvider.getAvailableImages().length);
        
        // Wait for image provider to load image list
        const maxWaitTime = 10000; // 10 seconds max wait
        const startTime = Date.now();
        
        while (!this.imageProvider.isReady() && (Date.now() - startTime) < maxWaitTime) {
            console.log(`Waiting for ImageProvider... (${(Date.now() - startTime) / 1000}s)`);
            await new Promise(resolve => setTimeout(resolve, 500));
        }
        
        if (this.imageProvider.isReady()) {
            console.log('ImageProvider ready, loading first image');
            console.log('Current image:', this.imageProvider.getCurrentImageName());
            await this.loadCurrentImage();
        } else {
            console.log('ImageProvider not ready after timeout, showing error');
            this.showNoImagesError();
        }
    }

    // Load current image using ImageProvider
    async loadCurrentImage() {
        try {
            if (!this.imageProvider.isReady()) {
                throw new Error('ImageProvider not ready');
            }

            console.log(`Loading real image: ${this.imageProvider.getCurrentImageName()}`);
            
            // Get image data from ImageProvider
            const imageData = await this.imageProvider.getCurrentImageData();
            
            // Update canvas
            this.canvas.width = imageData.width;
            this.canvas.height = imageData.height;
            
            // Draw image to main canvas
            const ctx = this.canvas.getContext('2d');
            ctx.putImageData(imageData.imageData, 0, 0);
            
            // Store current image data
            this.currentImage = imageData.imageData;
            
            // Update image processor
            this.imageProcessor.setOriginalImage(this.currentImage);
            
            // Clear previous face detections
            this.faceDetector.clearFaces();
            this.ui.disableAutoCrop();
            
            // Update UI
            this.ui.updateImageInfo(imageData.width, imageData.height);
            this.updateImageCounter();
            
            console.log(`✅ Successfully loaded REAL image: ${imageData.name} (${imageData.width}x${imageData.height})`);
            
        } catch (error) {
            console.error('Failed to load current image:', error);
            this.showNoImagesError();
        }
    }

    // Show error when no images are available
    showNoImagesError() {
        const ctx = this.canvas.getContext('2d');
        ctx.fillStyle = '#2c3e50';
        ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        ctx.fillStyle = '#ffffff';
        ctx.font = '16px Arial';
        ctx.textAlign = 'center';
        ctx.fillText('Real Images Not Found!', this.canvas.width/2, this.canvas.height/2 - 40);
        ctx.font = '12px Arial';
        ctx.fillText('Server Status: Check if running', this.canvas.width/2, this.canvas.height/2 - 20);
        ctx.fillText('Open: http://localhost:8080/simple-real-test.html', this.canvas.width/2, this.canvas.height/2);
        ctx.fillText('To verify server and images', this.canvas.width/2, this.canvas.height/2 + 20);
        ctx.fillText('Then reload this page', this.canvas.width/2, this.canvas.height/2 + 40);
        
        // Update UI
        this.ui.updateImageInfo(300, 300);
        if (this.ui.updateImageCounter) {
            this.ui.updateImageCounter();
        }
    }

    // Initialize demo navigation buttons
    initDemoNavigation() {
        const prevBtn = document.getElementById('prevImage');
        const nextBtn = document.getElementById('nextImage');
        
        if (prevBtn) {
            prevBtn.addEventListener('click', () => this.loadPreviousImage());
        }
        
        if (nextBtn) {
            nextBtn.addEventListener('click', () => this.loadNextImage());
        }
        
        // Add keyboard navigation
        document.addEventListener('keydown', (e) => {
            if (e.key === 'ArrowLeft') {
                this.loadPreviousImage();
            } else if (e.key === 'ArrowRight') {
                this.loadNextImage();
            }
        });
    }

    // Load next image using ImageProvider
    async loadNextImage() {
        if (this.imageProvider.nextImage()) {
            // Image change event will trigger loadCurrentImage automatically
            console.log('Next image requested');
        }
    }

    // Load previous image using ImageProvider
    async loadPreviousImage() {
        if (this.imageProvider.previousImage()) {
            // Image change event will trigger loadCurrentImage automatically
            console.log('Previous image requested');
        }
    }

    // Load image by filename (for recent files)
    loadImage(filename) {
        console.log(`Loading image: ${filename}`);
        
        // For recent files, we need to create a file input or use a different approach
        // Since we can't access real files directly, we'll generate appropriate synthetic image
        let generatedCanvas;
        
        if (filename.includes('portrait')) {
            generatedCanvas = this.imageGenerator.generatePortrait();
        } else if (filename.includes('street')) {
            generatedCanvas = this.imageGenerator.generateStreet();
        } else {
            generatedCanvas = this.imageGenerator.generatePortrait(); // Default
        }
        
        const img = new Image();
        img.onload = () => {
            // Update canvas
            this.canvas.width = img.width;
            this.canvas.height = img.height;
            
            // Draw image to main canvas
            const ctx = this.canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            
            // Get image data for processing
            const imageData = ctx.getImageData(0, 0, img.width, img.height);
            this.currentImage = imageData;
            
            // Update image processor
            this.imageProcessor.setOriginalImage(imageData);
            
            // Clear previous face detections
            this.faceDetector.clearFaces();
            this.ui.disableAutoCrop();
            
            // Update UI
            this.ui.updateImageInfo(img.width, img.height);
            
            console.log(`Loaded image: ${filename} (${img.width}x${img.height})`);
        };
        
        img.onerror = () => {
            console.error(`Failed to load image: ${filename}`);
            // Show fallback image instead
            this.loadCurrentImage();
        };
        
        img.src = generatedCanvas.toDataURL();
    }

    // Initialize drag and drop
    initDragAndDrop() {
        // Prevent default drag behaviors
        ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
            document.addEventListener(eventName, (e) => {
                e.preventDefault();
                e.stopPropagation();
            });
        });
        
        // Handle file drop
        document.addEventListener('drop', (e) => {
            const files = e.dataTransfer.files;
            if (files.length > 0) {
                this.loadRealFile(files[0]);
            }
        });
        
        // Add visual feedback for drag over
        document.addEventListener('dragover', () => {
            document.body.classList.add('drag-over');
        });
        
        document.addEventListener('dragleave', () => {
            document.body.classList.remove('drag-over');
        });
        
        document.addEventListener('drop', () => {
            document.body.classList.remove('drag-over');
        });
    }

    // Initialize canvas mouse events
    initCanvasEvents() {
        this.canvas.addEventListener('mousemove', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            this.mousePos.x = e.clientX - rect.left;
            this.mousePos.y = e.clientY - rect.top;
            this.updateMouseInfo();
        });

        this.canvas.addEventListener('mouseleave', () => {
            this.mousePos = { x: -1, y: -1 };
            this.updateMouseInfo();
        });
    }

    // Initialize file input
    initFileInput() {
        const fileInput = document.getElementById('fileInput');
        if (fileInput) {
            fileInput.addEventListener('change', (e) => {
                const file = e.target.files[0];
                if (file) {
                    this.loadRealFile(file);
                }
            });
        }
    }

    // Load real image file
    async loadRealFile(file) {
        try {
            console.log(`Loading real file: ${file.name} (${this.imageLoader.formatFileSize(file.size)})`);
            
            const result = await this.imageLoader.loadFromFile(file);
            
            // Create image element to draw on main canvas
            const img = new Image();
            img.onload = () => {
                // Update canvas dimensions
                this.canvas.width = img.width;
                this.canvas.height = img.height;
                
                // Draw image to main canvas
                const ctx = this.canvas.getContext('2d');
                ctx.drawImage(img, 0, 0);
                
                // Get image data for processing
                const imageData = ctx.getImageData(0, 0, img.width, img.height);
                this.currentImage = imageData;
                
                // Update image processor
                this.imageProcessor.setOriginalImage(imageData);
                
                // Clear previous face detections
                this.faceDetector.clearFaces();
                this.ui.disableAutoCrop();
                
                // Update UI
                this.ui.updateImageInfo(img.width, img.height);
                
                // Add to recent files
                this.addRecentFile(file.name);
                
                console.log(`Successfully loaded: ${file.name} (${img.width}x${img.height})`);
            };
            
            img.onerror = () => {
                throw new Error('Failed to load image data');
            };
            
            // Load the image from the data URL
            img.src = result.canvas.toDataURL();
            
        } catch (error) {
            console.error('Failed to load file:', error);
            alert(`Failed to load file: ${error.message}`);
        }
    }

    // Add file to recent files list
    addRecentFile(fileName) {
        if (!this.ui.recentFiles.includes(fileName)) {
            this.ui.recentFiles.unshift(fileName);
            // Keep only last 10 files
            if (this.ui.recentFiles.length > 10) {
                this.ui.recentFiles = this.ui.recentFiles.slice(0, 10);
            }
            this.ui.updateFileList();
        }
    }

    // Load initial synthetic image (fallback)
    loadInitialImage() {
        const generatedCanvas = this.imageGenerator.generatePortrait();
        const ctx = generatedCanvas.getContext('2d');
        const imageData = ctx.getImageData(0, 0, generatedCanvas.width, generatedCanvas.height);
        
        // Set to main canvas
        this.canvas.width = generatedCanvas.width;
        this.canvas.height = generatedCanvas.height;
        this.currentImage = imageData;
        
        // Initialize image processor with generated image
        this.imageProcessor.setOriginalImage(imageData);
        
        // Update UI
        this.ui.updateImageInfo(generatedCanvas.width, generatedCanvas.height);
        this.ui.enableAutoCrop(); // Enable auto-crop initially
        
        console.log('Loaded initial synthetic image');
    }

    // Load image by filename (for recent files)
    loadImage(filename) {
        console.log(`Loading image: ${filename}`);
        
        // For recent files, we need to create a file input or use a different approach
        // Since we can't access real files directly, we'll generate appropriate synthetic image
        let generatedCanvas;
        
        if (filename.includes('portrait')) {
            generatedCanvas = this.imageGenerator.generatePortrait();
        } else if (filename.includes('street')) {
            generatedCanvas = this.imageGenerator.generateStreet();
        } else {
            generatedCanvas = this.imageGenerator.generatePortrait(); // Default
        }
        
        const img = new Image();
        img.onload = () => {
            // Update canvas
            this.canvas.width = img.width;
            this.canvas.height = img.height;
            
            // Draw image to main canvas
            const ctx = this.canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            
            // Get image data for processing
            const imageData = ctx.getImageData(0, 0, img.width, img.height);
            this.currentImage = imageData;
            
            // Update image processor
            this.imageProcessor.setOriginalImage(imageData);
            
            // Clear previous face detections
            this.faceDetector.clearFaces();
            this.ui.disableAutoCrop();
            
            // Update UI
            this.ui.updateImageInfo(img.width, img.height);
            
            console.log(`Loaded image: ${filename} (${img.width}x${img.height})`);
        };
        
        img.onerror = () => {
            console.error(`Failed to load image: ${filename}`);
            // Show fallback image instead
            this.loadInitialImage();
        };
        
        img.src = generatedCanvas.toDataURL();
    }

    // Handle face detection completion
    onFacesDetected(faces) {
        console.log(`Detected ${faces.length} faces:`, faces);
        
        // Update UI panel with results
        if (this.ui) {
            this.ui.updateFaceDetectionResults(faces);
        }
        
        // Draw face boxes on canvas
        this.drawFaceBoxes(faces);
        
        // Enable auto-crop if faces found
        if (faces.length > 0) {
            this.ui.enableAutoCrop();
        } else {
            this.ui.disableAutoCrop();
        }
    }

    // Draw face detection boxes
    drawFaceBoxes(faces) {
        const ctx = this.canvas.getContext('2d');
        
        // Save current composite operation
        const prevOperation = ctx.globalCompositeOperation;
        ctx.globalCompositeOperation = 'source-over';
        
        faces.forEach(face => {
            // Draw bounding box
            ctx.strokeStyle = '#00ff00';
            ctx.lineWidth = 2;
            ctx.strokeRect(face.x, face.y, face.width, face.height);
            
            // Draw confidence score
            ctx.fillStyle = '#00ff00';
            ctx.font = '12px monospace';
            ctx.fillText(
                `${(face.confidence * 100).toFixed(1)}%`,
                face.x,
                face.y - 5
            );
        });
        
        // Restore composite operation
        ctx.globalCompositeOperation = prevOperation;
    }

    // Handle split position change
    onSplitPositionChanged(position) {
        console.log(`Split position changed: ${position}`);
        
        // Update right canvas with processed image
        if (this.rightCanvas && this.imageProcessor) {
            const rightCtx = this.rightCanvas.getContext('2d');
            const processedData = this.imageProcessor.getCurrentImageData();
            
            if (processedData) {
                rightCtx.putImageData(processedData, 0, 0);
            }
        }
    }

    // Update mouse position info
    updateMouseInfo() {
        if (this.mousePos.x >= 0 && this.mousePos.y >= 0 && this.currentImage) {
            const pixelIndex = (Math.floor(this.mousePos.y) * this.canvas.width + Math.floor(this.mousePos.x)) * 4;
            const r = this.currentImage.data[pixelIndex];
            const g = this.currentImage.data[pixelIndex + 1];
            const b = this.currentImage.data[pixelIndex + 2];
            
            // Update status with RGB values (you could add this to status bar)
            console.log(`RGB at (${this.mousePos.x}, ${this.mousePos.y}): R=${r}, G=${g}, B=${b}`);
        }
    }

    // Handle keyboard shortcuts
    handleKeyboardShortcuts(event) {
        // Additional keyboard shortcuts can be handled here
        if (event.ctrlKey || event.metaKey) {
            switch (event.key) {
                case 'r':
                    event.preventDefault();
                    this.resetImage();
                    break;
                case 's':
                    event.preventDefault();
                    this.saveImage();
                    break;
            }
        }
    }

    // Reset image to original
    resetImage() {
        if (this.imageProcessor) {
            this.imageProcessor.reset();
        }
    }

    // Save current image
    saveImage() {
        const link = document.createElement('a');
        link.download = 'glance-demo-image.png';
        link.href = this.canvas.toDataURL();
        link.click();
    }
}

// Initialize app when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new App();
});
