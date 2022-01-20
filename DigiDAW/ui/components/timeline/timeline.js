class TimelineDrawer extends Element {
    app;

    drawTimeline(graphics) {

    }

    this(props, kids) {
        this.app = props.app;
        this.paintBackground = this.drawTimeline;
    }

    render() {
        this.requestPaint();
        return <section style="width: *; height: *;"></section>
    }
}


export class Timeline extends Element {
    app;

    this(props, kids) {
        this.app = props.app;
    }

    render() {
        return <section style="overflow-y: auto; flow: horizontal;">
            <div style="width: 400px; height: *; flow: vertical; background-color: var(panel-back);">

            </div>
            <div style="width: *; height: *; flow: horizontal; background-color: var(back);">
                <TimelineDrawer app={this.app} />
            </div>
        </section>
    }
}