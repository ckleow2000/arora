(extension = {
    name : "Custom UserAgents",
    version : "1.0",
    description : "Sets the browser user agent to a custom user agent.",
    icon : "useragent.svg",

    userAgent : "",

    MainWindow_ToolsMenu : function() {
        return {
            name   : "User Agent",
            action : [
                {
                    name : "Default",
                    action : function() { extension.userAgent = ""; }
                },
                { separator : true },
                {
                    name : "Internet Explorer 6.0",
                    action : function() {
                        extension.userAgent = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)";
                        print ("Chaning!");
                        extension.userAgentForUrl();
                    }
                }
            ]
        }
    },

    userAgentForUrl : function() {
        return this.userAgent;
    }

})
