export class StatusBar extends Element {
    app;

    this(props, kids) {
        this.app = props.app;
    }

    render() {
        return <section styleset={__DIR__ + "statusbar.css#bar"}>
            Here goes your status
        </section>;
    }
}