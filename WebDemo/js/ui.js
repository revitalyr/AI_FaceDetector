// UI Controller - Manages sliders, status bar, and UI interactions
class UIController {
    constructor() {
        this.sliders = {
            exposure: document.getElementById('exposureSlider'),
            contrast: document.getElementById('contrastSlider'),
            gamma: document.getElementById('gammaSlider'),
            saturation: document.getElementById('saturationSlider')
        };
        
        this.valueDisplays = {
            exposure: document.getElementById('exposureValue'),
            contrast: document.getElementById('contrastValue'),
            gamma: document.getElementById('gammaValue'),
            saturation: document.getElementById('saturationValue')
        };
        
        this.statusElements = {
            imageInfo: document.getElementById('imageInfo'),
            zoomLevel: document.getElementById('zoomLevel'),
            gpuIndicator: document.getElementById('gpuIndicator')
        };
        
        // Face detection panel elements
        this.detectionElements = {
            statusIndicator: document.getElementById('statusIndicator'),
            statusText: document.getElementById('statusText'),
            detectionResults: document.getElementById('detectionResults'),
            facesCount: document.getElementById('facesCount'),
            facesList: document.getElementById('facesList'),
            detectBtn: document.getElementById('detectFaces'),
            clearBtn: document.getElementById('clearDetection')
        };
        
        this.buttons = {
            splitMode: document.getElementById('splitMode'),
            autoCrop: document.getElementById('autoCrop'),
            openFile: document.getElementById('openFile')
        };
        
        this.fileList = document.getElementById('fileList');
        this.recentFiles = [];
        
        this.zoom = 1.0;
        this.frameCount = 0;
        this.lastFrameTime = performance.now();
        
        this.initEventListeners();
        this.initRecentFiles();
        this.startFPSCounter();
        this.initFaceDetectionPanel();
    }

    // Initialize event listeners
    initEventListeners() {
        // Slider events
        Object.keys(this.sliders).forEach(key => {
            this.sliders[key].addEventListener('input', (e) => {
                this.updateValueDisplay(key, e.target.value);
                this.onSliderChange(key, e.target.value);
            });
        });

        // Button events
        this.buttons.splitMode.addEventListener('click', () => this.onSplitModeToggle());
        this.buttons.autoCrop.addEventListener('click', () => this.onAutoCrop());
        this.buttons.openFile.addEventListener('click', () => this.onOpenFile());

        // Face detection panel events
        if (this.detectionElements.detectBtn) {
            this.detectionElements.detectBtn.addEventListener('click', () => this.onDetectFaces());
        }
        if (this.detectionElements.clearBtn) {
            this.detectionElements.clearBtn.addEventListener('click', () => this.onClearDetection());
        }

        // Keyboard events
        document.addEventListener('keydown', (e) => this.onKeyDown(e));
    }

    // Initialize face detection panel
    initFaceDetectionPanel() {
        this.updateDetectionStatus('ready', 'Ready to detect faces');
    }

    // Update detection status
    updateDetectionStatus(status, message) {
        if (!this.detectionElements.statusIndicator || !this.detectionElements.statusText) return;
        
        this.detectionElements.statusText.textContent = message;
        
        // Remove all status classes
        this.detectionElements.statusIndicator.classList.remove('detecting', 'success', 'error');
        
        // Add appropriate class
        switch(status) {
            case 'detecting':
                this.detectionElements.statusIndicator.classList.add('detecting');
                break;
            case 'success':
                this.detectionElements.statusIndicator.classList.add('success');
                break;
            case 'error':
                this.detectionElements.statusIndicator.classList.add('error');
                break;
            default:
                // ready state - no special class
                break;
        }
    }

    // Update face detection results
    updateFaceDetectionResults(faces) {
        if (!this.detectionElements.detectionResults || !this.detectionElements.facesCount || !this.detectionElements.facesList) return;
        
        if (faces.length === 0) {
            this.detectionElements.detectionResults.style.display = 'none';
            this.updateDetectionStatus('success', 'No faces detected');
            return;
        }
        
        // Show results
        this.detectionElements.detectionResults.style.display = 'block';
        this.detectionElements.facesCount.textContent = faces.length;
        
        // Clear previous results
        this.detectionElements.facesList.innerHTML = '';
        
        // Add face items
        faces.forEach((face, index) => {
            const faceItem = document.createElement('div');
            faceItem.className = 'face-item';
            
            const faceInfo = document.createElement('span');
            faceInfo.className = 'face-info';
            faceInfo.textContent = `Face ${index + 1}: ${Math.round(face.width)}×${Math.round(face.height)}px`;
            
            const confidence = document.createElement('span');
            confidence.className = 'confidence';
            confidence.textContent = `${Math.round(face.confidence * 100)}%`;
            
            faceItem.appendChild(faceInfo);
            faceItem.appendChild(confidence);
            this.detectionElements.facesList.appendChild(faceItem);
        });
        
        this.updateDetectionStatus('success', `Detected ${faces.length} face${faces.length > 1 ? 's' : ''}`);
    }

    // Clear face detection results
    clearFaceDetectionResults() {
        if (!this.detectionElements.detectionResults || !this.detectionElements.facesList) return;
        
        this.detectionElements.detectionResults.style.display = 'none';
        this.detectionElements.facesList.innerHTML = '';
        this.updateDetectionStatus('ready', 'Ready to detect faces');
    }

    // Update image counter display
    updateImageCounter() {
        const counter = document.getElementById('imageCounter');
        if (counter && window.app && window.app.imageProvider && window.app.imageProvider.isReady()) {
            const current = window.app.imageProvider.getCurrentIndex() + 1;
            const total = window.app.imageProvider.getTotalImages();
            counter.textContent = `${current}/${total}`;
        }
    }

    // Initialize recent files list
    initRecentFiles() {
        // Start with empty list - will be populated when files are loaded
        this.recentFiles = [];
        this.updateFileList();
    }

    // Update value display for slider
    updateValueDisplay(slider, value) {
        if (this.valueDisplays[slider]) {
            this.valueDisplays[slider].textContent = parseFloat(value).toFixed(1);
        }
    }

    // Handle slider change
    onSliderChange(slider, value) {
        if (window.app && window.app.imageProcessor) {
            switch (slider) {
                case 'exposure':
                    window.app.imageProcessor.setExposure(value);
                    break;
                case 'contrast':
                    window.app.imageProcessor.setContrast(value);
                    break;
                case 'gamma':
                    window.app.imageProcessor.setGamma(value);
                    break;
                case 'saturation':
                    window.app.imageProcessor.setSaturation(value);
                    break;
            }
        }
    }

    // Toggle split mode
    onSplitModeToggle() {
        if (window.app && window.app.splitScreen) {
            const isActive = this.buttons.splitMode.classList.contains('active');
            
            if (isActive) {
                window.app.splitScreen.disableSplitMode();
                this.buttons.splitMode.classList.remove('active');
            } else {
                window.app.splitScreen.enableSplitMode();
                this.buttons.splitMode.classList.add('active');
            }
        }
    }

    // Handle face detection
    onDetectFaces() {
        if (window.app && window.app.faceDetector && window.app.imageProcessor) {
            // Update status to detecting
            this.updateDetectionStatus('detecting', 'Detecting faces...');
            
            const imageData = window.app.imageProcessor.getCurrentImageData();
            window.app.faceDetector.detectFaces(imageData);
        }
    }

    // Handle clear detection
    onClearDetection() {
        if (window.app && window.app.faceDetector) {
            window.app.faceDetector.clearFaces();
            this.clearFaceDetectionResults();
            this.disableAutoCrop();
            
            // Clear face boxes from canvas
            if (window.app.canvas) {
                const ctx = window.app.canvas.getContext('2d');
                // Redraw the current image without face boxes
                if (window.app.currentImage) {
                    ctx.putImageData(window.app.currentImage, 0, 0);
                }
            }
        }
    }

    // Handle auto crop
    onAutoCrop() {
        if (window.app && window.app.faceDetector) {
            const bounds = window.app.faceDetector.getFaceBounds();
            if (bounds) {
                this.animateZoomToBounds(bounds);
            }
        }
    }

    // Animate zoom to bounds with Canvas scaling
    animateZoomToBounds(bounds) {
        const canvas = document.getElementById('mainCanvas');
        if (!canvas) return;
        
        const scaleX = canvas.width / bounds.width;
        const scaleY = canvas.height / bounds.height;
        const targetZoom = Math.min(scaleX, scaleY) * 0.8; // 80% of bounds
        
        const startZoom = this.zoom;
        const duration = 500; // ms
        const startTime = performance.now();
        
        const animate = () => {
            const elapsed = performance.now() - startTime;
            const progress = Math.min(elapsed / duration, 1);
            
            // Easing function
            const easeProgress = 1 - Math.pow(1 - progress, 3);
            this.zoom = startZoom + (targetZoom - startZoom) * easeProgress;
            
            this.updateZoomDisplay();
            
            if (progress < 1) {
                requestAnimationFrame(animate);
            }
        };
        
        animate();
    }

    // Handle open file
    onOpenFile() {
        // Trigger file input click
        const fileInput = document.getElementById('fileInput');
        if (fileInput) {
            fileInput.click();
        }
    }

    // Handle keyboard events
    onKeyDown(event) {
        // Ctrl+F or Ctrl+0 for zoom
        if (event.ctrlKey || event.metaKey) {
            switch (event.key) {
                case 'f':
                    event.preventDefault();
                    this.zoom = 1.0;
                    this.updateZoomDisplay();
                    break;
                case '0':
                    event.preventDefault();
                    this.zoom = 1.0;
                    this.updateZoomDisplay();
                    break;
                case '=':
                case '+':
                    event.preventDefault();
                    this.zoom = Math.min(this.zoom * 1.2, 5.0);
                    this.updateZoomDisplay();
                    break;
                case '-':
                case '_':
                    event.preventDefault();
                    this.zoom = Math.max(this.zoom / 1.2, 0.1);
                    this.updateZoomDisplay();
                    break;
            }
        }
    }

    // Update zoom display
    updateZoomDisplay() {
        if (this.statusElements.zoomLevel) {
            this.statusElements.zoomLevel.textContent = `Zoom: ${Math.round(this.zoom * 100)}%`;
        }
    }

    // Update image info display
    updateImageInfo(width, height, colorSpace = 'RGB') {
        if (this.statusElements.imageInfo) {
            const fps = this.getFPS();
            this.statusElements.imageInfo.textContent = `${width}×${height} • ${colorSpace} • ${fps} FPS`;
        }
    }

    // FPS counter
    startFPSCounter() {
        setInterval(() => {
            this.frameCount++;
            const now = performance.now();
            if (now - this.lastFrameTime >= 1000) {
                if (this.statusElements.imageInfo) {
                    const currentText = this.statusElements.imageInfo.textContent;
                    const parts = currentText.split(' • ');
                    if (parts.length >= 3) {
                        parts[2] = `${this.frameCount} FPS`;
                        this.statusElements.imageInfo.textContent = parts.join(' • ');
                    }
                }
                this.frameCount = 0;
                this.lastFrameTime = now;
            }
        }, 16); // ~60fps
    }

    // Get current FPS
    getFPS() {
        return this.frameCount;
    }

    // Update file list display
    updateFileList() {
        if (!this.fileList) return;
        
        this.fileList.innerHTML = '';
        this.recentFiles.forEach(fileName => {
            const fileItem = document.createElement('div');
            fileItem.className = 'file-item';
            fileItem.textContent = fileName;
            fileItem.addEventListener('click', () => {
                if (window.app) {
                    window.app.loadImage(fileName);
                }
            });
            this.fileList.appendChild(fileItem);
        });
    }

    // Enable auto-crop button
    enableAutoCrop() {
        if (this.buttons.autoCrop) {
            this.buttons.autoCrop.disabled = false;
        }
    }

    // Disable auto-crop button
    disableAutoCrop() {
        if (this.buttons.autoCrop) {
            this.buttons.autoCrop.disabled = true;
        }
    }

    // Update GPU indicator
    updateGPUIndicator(isGPU = false) {
        if (this.statusElements.gpuIndicator) {
            this.statusElements.gpuIndicator.textContent = isGPU ? 'GPU: Active' : 'GPU: Software';
        }
    }
}

// Export for use
window.UIController = UIController;
