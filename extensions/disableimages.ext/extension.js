(extension = {
    name : "Extra Tools",
    description : "Some extra actions for the Tools menu",

    MainWindow_ToolsMenu : function() {
        return [
            { separator : true },
            {
                name   : "Disable Images",

                settings : QWebSettings.globalSettings(),

                checked : function() {
                    return settings.testAttribute(QWebSettings.AutoLoadImages);
                },

                action : function() {
                    var loadImages = this.settings.testAttribute(QWebSettings.AutoLoadImages);
                    this.settings.setAttribute(QWebSettings.AutoLoadImages, !loadImages);
                }
            },
        ]
    }

   /*
   MainWindow_Tools2Menu : function() {
        var allActions = new Array();
        allActions.append(x);

        var x = new QAction(this);
        x.setSeperator(true);

        var b = QAction("Disable Images", this);
        b.settings : QWebSettings.globalSettings();
        b.triggered.connect(this, function() {
                var loadImages = this.settings.testAttribute(QWebSettings.AutoLoadImages);
                this.settings.setAttribute(QWebSettings.AutoLoadImages, !loadImages);
                });
        b.checkable(true);
        b.setChecked(settings.testAttribute(QWebSettings.AutoLoadImages));
        allActions.append(b);

        var c = new QAction(this);
        c.setSeperator(true);
        allActions.append(c);

        return allActions;
    }*/

})
