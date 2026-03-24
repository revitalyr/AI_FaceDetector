// Image Generator - Creates synthetic portrait using Canvas API
class ImageGenerator {
    constructor() {
        this.width = 800;
        this.height = 600;
        this.canvas = null;
        this.ctx = null;
    }

    // Generate synthetic portrait
    generatePortrait() {
        const canvas = document.createElement('canvas');
        canvas.width = this.width;
        canvas.height = this.height;
        const ctx = canvas.getContext('2d');

        // Create gradient background
        const gradient = ctx.createLinearGradient(0, 0, 0, this.height);
        gradient.addColorStop(0, '#2c3e50');
        gradient.addColorStop(0.5, '#3498db');
        gradient.addColorStop(1, '#2c3e50');
        ctx.fillStyle = gradient;
        ctx.fillRect(0, 0, this.width, this.height);

        // Draw face outline
        this.drawFace(ctx, this.width / 2, this.height / 2 - 50);

        // Draw some features
        this.drawFeatures(ctx, this.width / 2, this.height / 2 - 50);

        // Add noise for realism
        this.addNoise(ctx);

        return canvas;
    }

    // Draw basic face shape
    drawFace(ctx, centerX, centerY) {
        // Face outline
        ctx.beginPath();
        ctx.ellipse(centerX, centerY, 120, 150, 0, 0, 2 * Math.PI);
        ctx.fillStyle = '#fdbcb4';
        ctx.fill();
        ctx.strokeStyle = '#f4a460';
        ctx.lineWidth = 2;
        ctx.stroke();

        // Eyes
        ctx.beginPath();
        ctx.ellipse(centerX - 30, centerY - 20, 15, 8, 0, 0, 2 * Math.PI);
        ctx.fillStyle = '#333';
        ctx.fill();

        ctx.beginPath();
        ctx.ellipse(centerX + 30, centerY - 20, 15, 8, 0, 0, 2 * Math.PI);
        ctx.fillStyle = '#333';
        ctx.fill();

        // Nose
        ctx.beginPath();
        ctx.moveTo(centerX, centerY - 10);
        ctx.lineTo(centerX - 8, centerY + 10);
        ctx.lineTo(centerX + 8, centerY + 10);
        ctx.closePath();
        ctx.strokeStyle = '#e8a298';
        ctx.lineWidth = 2;
        ctx.stroke();

        // Mouth
        ctx.beginPath();
        ctx.arc(centerX, centerY + 30, 20, 0, 0, Math.PI);
        ctx.strokeStyle = '#e74c3c';
        ctx.lineWidth = 3;
        ctx.stroke();
    }

    // Draw additional features
    drawFeatures(ctx, centerX, centerY) {
        // Hair
        ctx.beginPath();
        ctx.ellipse(centerX, centerY - 100, 100, 60, 0, 0, 2 * Math.PI);
        ctx.fillStyle = '#3e2723';
        ctx.fill();

        // Ears
        ctx.beginPath();
        ctx.ellipse(centerX - 120, centerY, 25, 40, -0.3, 0, 2 * Math.PI);
        ctx.fillStyle = '#fdbcb4';
        ctx.fill();

        ctx.beginPath();
        ctx.ellipse(centerX + 120, centerY, 25, 40, 0.3, 0, 2 * Math.PI);
        ctx.fillStyle = '#fdbcb4';
        ctx.fill();

        // Clothing
        ctx.fillStyle = '#3498db';
        ctx.fillRect(centerX - 150, centerY + 100, 300, 200);
    }

    // Add noise texture
    addNoise(ctx) {
        const imageData = ctx.getImageData(0, 0, this.width, this.height);
        const data = imageData.data;
        
        for (let i = 0; i < data.length; i += 4) {
            const noise = (Math.random() - 0.5) * 20;
            data[i] = Math.max(0, Math.min(255, data[i] + noise));     // R
            data[i + 1] = Math.max(0, Math.min(255, data[i + 1] + noise)); // G
            data[i + 2] = Math.max(0, Math.min(255, data[i + 2] + noise)); // B
        }
        
        ctx.putImageData(imageData, 0, 0);
    }

    // Generate different image types
    generateStreet() {
        const canvas = document.createElement('canvas');
        canvas.width = this.width;
        canvas.height = this.height;
        const ctx = canvas.getContext('2d');

        // Street scene
        const gradient = ctx.createLinearGradient(0, 0, 0, this.height);
        gradient.addColorStop(0, '#87ceeb');
        gradient.addColorStop(1, '#98d8c8');
        ctx.fillStyle = gradient;
        ctx.fillRect(0, 0, this.width, this.height);

        // Buildings
        this.drawBuildings(ctx);

        return canvas;
    }

    drawBuildings(ctx) {
        const buildings = [
            { x: 100, y: 200, w: 80, h: 300, color: '#34495e' },
            { x: 250, y: 150, w: 100, h: 350, color: '#2c3e50' },
            { x: 400, y: 250, w: 90, h: 250, color: '#e74c3c' },
            { x: 550, y: 180, w: 110, h: 320, color: '#f39c12' }
        ];

        buildings.forEach(building => {
            ctx.fillStyle = building.color;
            ctx.fillRect(building.x, building.y, building.w, building.h);
            
            // Windows
            ctx.fillStyle = '#ffeaa7';
            for (let wy = building.y + 20; wy < building.y + building.h - 20; wy += 30) {
                for (let wx = building.x + 10; wx < building.x + building.w - 10; wx += 20) {
                    ctx.fillRect(wx, wy, 8, 12);
                }
            }
        });
    }
}

// Export for use
window.ImageGenerator = ImageGenerator;
