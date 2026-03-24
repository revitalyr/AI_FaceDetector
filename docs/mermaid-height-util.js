class MermaidHeightAdjuster {
    constructor() {
        this.currentScale = 1;
        this.renderedTabs = new Set();
    }

    setupTabSwitching() {
        document.querySelectorAll('.nav-tab').forEach(tab => {
            tab.addEventListener('click', () => {
                document.querySelectorAll('.nav-tab').forEach(t => t.classList.remove('active'));
                tab.classList.add('active');
                const tabId = tab.getAttribute('data-tab');
                this.renderAndScale(tabId);
            });
        });
    }

    setupScaleControls() {
        document.addEventListener('click', (event) => {
            const target = event.target;
            if (target.classList.contains('scale-button')) {
                if (target.hasAttribute('data-scale-delta')) {
                    const delta = parseInt(target.getAttribute('data-scale-delta'), 10);
                    this.changeScale(delta);
                } else if (target.hasAttribute('data-reset-scale')) {
                    this.resetScale();
                }
            }
        });
    }

    changeScale(delta) {
        const newScale = Math.max(0.5, Math.min(2.0, this.currentScale + delta / 100));
        this.setScale(newScale);
    }

    resetScale() {
        this.setScale(1);
    }

    setScale(scale) {
        this.currentScale = scale;
        this.updateScaleDisplay();
        this.renderedTabs.forEach(tabId => {
            this.applyScaleToDiagram(tabId);
        });
    }

    updateScaleDisplay() {
        const activeTab = document.querySelector('.tab-content.active');
        if (!activeTab) return;
        const scaleValueEl = activeTab.querySelector('.scale-value');
        if (scaleValueEl) {
            scaleValueEl.textContent = `${Math.round(this.currentScale * 100)}%`;
        }
    }

    async renderAndScale(tabId) {
        document.querySelectorAll('.tab-content').forEach(tc => tc.classList.remove('active'));
        const tabContent = document.getElementById(tabId);
        if (tabContent) tabContent.classList.add('active');

        const mermaidEl = document.getElementById(`${tabId}-diagram`);
        if (!mermaidEl) return;

        const diagramContainer = mermaidEl.closest('.diagram-container');

        if (this.renderedTabs.has(tabId)) {
            this.applyScaleToDiagram(tabId);
            return;
        }

        const placeholder = diagramContainer ? diagramContainer.querySelector('.mermaid-placeholder') : null;
        if (placeholder) placeholder.style.display = 'block';

        setTimeout(async () => {
            try {
                await mermaid.run({ nodes: [mermaidEl] });
                this.renderedTabs.add(tabId);
                if (placeholder) placeholder.style.display = 'none';
                this.applyScaleToDiagram(tabId);
            } catch (error) {
                console.error(`Error rendering diagram for tab ${tabId}:`, error);
                if (placeholder) placeholder.innerHTML = `<div style="padding:20px;text-align:center;color:#dc3545;">Error rendering diagram: ${error.message}</div>`;
            }
        }, 200);
    }

    applyScaleToDiagram(tabId) {
        const mermaidEl = document.getElementById(`${tabId}-diagram`);
        if (!mermaidEl) return;
        const svg = mermaidEl.querySelector('svg');
        if (!svg) return;

        const diagramContainer = mermaidEl.closest('.diagram-container');
        svg.style.transform = `scale(${this.currentScale})`;
        svg.style.transformOrigin = 'top left';
        if (diagramContainer) this.adjustDiagramContainer(diagramContainer);
    }

    adjustDiagramContainer(container) {
        const svg = container.querySelector('svg');
        if (!svg) return;
        const bbox = svg.getBoundingClientRect();
        container.style.minWidth = `${Math.max(bbox.width, 600)}px`;
        container.style.minHeight = `${Math.max(bbox.height, 300)}px`;
    }

    renderDiagram(tabId) {
        this.renderAndScale(tabId);
    }
}
