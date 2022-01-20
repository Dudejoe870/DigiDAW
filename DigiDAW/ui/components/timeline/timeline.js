export class Timeline extends Element {
    app;

    this(props, kids) {
        this.app = props.app;
    }

    render() {
        return <section>
            <div style="width: 400px; height: *; flow: vertical; background-color: var(panel-back);">
            </div>
        </section>
    }
}