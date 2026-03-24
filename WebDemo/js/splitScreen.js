// Split Screen functionality with draggable divider
class SplitScreen {
    constructor(mainCanvas, overlayCanvas) {
        this.mainCanvas = mainCanvas;
        this.overlayCanvas = overlayCanvas;
        this.overlayCtx = overlayCanvas.getContext('2d');
        this.isSplitMode = false;
        this.isDragging = false;
        this.splitPosition = 0.5; // Normalized position (0-1)
        
        this.initEventListeners();
    }

    // Initialize mouse event listeners
    initEventListeners() {
        this.mainCanvas.addEventListener('mousedown', this.onMouseDown.bind(this));
        document.addEventListener('mousemove', this.onMouseMove.bind(this));
        document.addEventListener('mouseup', this.onMouseUp.bind(this));
    }

    // Enable split mode
    enableSplitMode() {
        this.isSplitMode = true;
        this.overlayCanvas.style.display = 'block';
        this.updateSplitPosition();
    }

    // Disable split mode
    disableSplitMode() {
        this.isSplitMode = false;
        this.overlayCanvas.style.display = 'none';
    }

    // Handle mouse down
    onMouseDown(event) {
        if (!this.isSplitMode) return;
        
        const rect = this.mainCanvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const normalizedX = x / rect.width;
        
        // Check if near the split line (within 10px)
        const splitX = this.splitPosition * rect.width;
        if (Math.abs(x - splitX) < 10) {
            this.isDragging = true;
            event.preventDefault();
        }
    }

    // Handle mouse move
    onMouseMove(event) {
        if (!this.isDragging || !this.isSplitMode) return;
        
        const rect = this.mainCanvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        this.splitPosition = Math.max(0.1, Math.min(0.9, x / rect.width));
        
        this.updateSplitPosition();
    }

    // Handle mouse up
    onMouseUp(event) {
        this.isDragging = false;
    }

    // Update split line position
    updateSplitPosition() {
        const rect = this.mainCanvas.getBoundingClientRect();
        const splitX = this.splitPosition * rect.width;
        
        // Update overlay position
        this.overlayCanvas.style.left = `${splitX}px`;
        this.overlayCanvas.style.transform = `translateX(-50%)`;
        
        // Update right canvas size
        this.overlayCanvas.width = rect.width - splitX;
        this.overlayCanvas.height = rect.height;
        
        // Trigger split position update
        if (window.app) {
            window.app.onSplitPositionChanged(this.splitPosition);
        }
    }

    // Get split position
    getSplitPosition() {
        return this.splitPosition;
    }

    // Set split position
    setSplitPosition(position) {
        this.splitPosition = Math.max(0.1, Math.min(0.9, position));
        this.updateSplitPosition();
    }
}

// Export for use
window.SplitScreen = SplitScreen;
