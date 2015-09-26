var currentConversation = 0;
var urlPrefix = "http://localhost/msg/v1/html/";
var timerId = 0;

function gotoConversation(conv_id)
{
    window.clearTimeout(timerId);
    currentConversation = parseInt(conv_id.substring(5));
    //alert("Going to conversation " + currentConversation);
    displayStuff(currentConversation);
}

function updateLinks()
{
    alert("Updating links");
    // update onclicks
    $("span.conversation").each(function(index) {
        $(this).click(function() {
            gotoConversation($(this).attr("id"));})
    });
}

function displayStuff()
{
    // clear old data
    $("#conversations").text("");
    $("#messages").text("");

    // conversations window
    var conversationsUrl = urlPrefix + "conversations/all";
    $("#conversations").load(conversationsUrl);

    var imUrl;
    if (currentConversation == 0) {
        imUrl = urlPrefix + "messages/all";
    } else {
        imUrl = urlPrefix + "conversations/" + currentConversation;
    }
    alert(imUrl);
    $("#messages").load(imUrl);

    if (currentConversation > 0) {
        // enable input
        $("#send_msg").show()
    } else {
        // disable input
        $("#send_msg").hide()
    }

    updateLinks();
    timerId = window.setTimeout(displayStuff, 10000);
}

displayStuff();
