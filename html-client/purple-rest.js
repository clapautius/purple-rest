var currentConversation = 0;
var urlPrefix = "../msg/v1/html/";
var timerId = 0;

function gotoConversation(conv_id)
{
    if (timerId > 0) {
        window.clearTimeout(timerId);
        timerId = 0;
    }
    currentConversation = parseInt(conv_id.substring(5));
    console.log("Going to conversation " + currentConversation);
    displayConversations();
}


function updateLinks()
{
    console.log("Updating links");
    // update onclicks
    $("span.conversation").each(function(index) {
        console.log("Setting onclick for conv id " + $(this).attr("id"));
        $(this).click(function() {
            gotoConversation($(this).attr("id"));})
    });

    if (currentConversation > 0) {
        // enable input
        $("#send_msg").show()
    } else {
        // disable input
        $("#send_msg").hide()
    }

    timerId = window.setTimeout(displayConversations, 10000);
}


function displayMessages()
{
    var imUrl;
    if (currentConversation == 0) {
        imUrl = urlPrefix + "messages/all";
    } else {
        imUrl = urlPrefix + "conversations/" + currentConversation;
    }
    console.log(imUrl);
    $("#messages").load(imUrl, updateLinks);

}


function displayConversations()
{
    // clear old data
    $("#conversations").text("");
    $("#messages").text("");

    // conversations window
    var conversationsUrl = urlPrefix + "conversations/all";
    $("#conversations").load(conversationsUrl, displayMessages);

}


function sendMessageResult(data)
{
    $("#send_msg_text").val("");
    // wait 1 sec. for the message to be processed
    window.setTimeout(displayConversations, 1000);
}


function sendMessageToPurple()
{
    if (timerId > 0) {
        window.clearTimeout(timerId);
        timerId = 0;
    }
    postUrl = urlPrefix + "conversations/" + currentConversation;
    message = $("#send_msg_text").val();
    console.log("Sending message " + message + " to address " + postUrl);
    $.post(postUrl, message, function(data, textStatus) {
        console.log(textStatus);
        sendMessageResult(data);
    });
}

displayConversations();
