// Histogram calculation and rendering
class Histogram {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.data = null;
    }

    // Update histogram with new image data
    updateHistogram(imageData) {
        if (!imageData || !imageData.data) {
            console.log('Histogram: No image data provided');
            return;
        }
        
        const data = imageData.data;
        const histogram = {
            r: new Array(256).fill(0),
            g: new Array(256).fill(0),
            b: new Array(256).fill(0),
            luminance: new Array(256).fill(0)
        };

        // Calculate histogram
        for (let i = 0; i < data.length; i += 4) {
            const r = data[i];
            const g = data[i + 1];
            const b = data[i + 2];
            
            histogram.r[r]++;
            histogram.g[g]++;
            histogram.b[b]++;
            
            // Calculate luminance
            const luminance = Math.round(0.299 * r + 0.587 * g + 0.114 * b);
            histogram.luminance[luminance]++;
        }

        this.data = histogram;
        this.render();
    }

    // Render histogram to canvas
    render() {
        if (!this.data) return;

        const width = this.canvas.width;
        const height = this.canvas.height;
        
        // Clear canvas
        this.ctx.clearRect(0, 0, width, height);
        
        // Find max value for normalization
        const maxValue = Math.max(
            ...this.data.r,
            ...this.data.g,
            ...this.data.b,
            ...this.data.luminance
        );

        // Draw histogram
        this.drawChannel(this.data.r, '#ff6b6b', 0, maxValue);
        this.drawChannel(this.data.g, '#4ecdc4', height / 3, maxValue);
        this.drawChannel(this.data.b, '#45b7d1', 2 * height / 3, maxValue);
        
        // Draw luminance as overlay
        this.ctx.globalAlpha = 0.3;
        this.drawChannel(this.data.luminance, '#ffffff', 0, maxValue);
        this.ctx.globalAlpha = 1.0;
    }

    // Draw single channel
    drawChannel(data, color, yOffset, maxValue) {
        const width = this.canvas.width;
        const height = this.canvas.height / 3;
        const barWidth = width / 256;
        
        this.ctx.fillStyle = color;
        
        for (let i = 0; i < 256; i++) {
            const barHeight = (data[i] / maxValue) * height;
            this.ctx.fillRect(
                i * barWidth,
                yOffset + height - barHeight,
                barWidth - 1, // Leave 1px gap between bars
                barHeight
            );
        }
    }

    // Get histogram data
    getData() {
        return this.data;
    }
}

// Export for use
window.Histogram = Histogram;
