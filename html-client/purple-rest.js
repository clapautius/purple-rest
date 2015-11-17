var currentConversation = 0;
var urlPrefix = "rest/v1/html/";
var urlPrefixJson = "rest/v1/json/";
var timerId = 0;
var maxCurrentId = -1;
var jsonSuccess = false;


function displayError(errorMsg)
{
    console.log("Some error happened");
    $("#status-bar-2").html("<span style=\"color: red;\">" + errorMsg + "</span>");
}


function displayRefresh()
{
    if (jsonSuccess) {
        var now = new Date(Date.now());
        var formatted = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
        $("#status-bar-2").html("Last update: " + formatted);
    }
}


function mobileLayout()
{
    $("#content").css("width", "100%");
}


function clearHistory()
{
    $("#messages").text("");
    if (timerId > 0) {
        window.clearTimeout(timerId);
        timerId = window.setTimeout(areThereNewMessagesP, 10000);
    }
    maxCurrentId = -1;
    var clearMsgCmd = urlPrefixJson + "cmd/clear_history";
    $.ajax({
        url: clearMsgCmd,
        dataType: 'json',
        success: function(data) {
            jsonSuccess = true;
            displayRefresh();
        },
        error: function(data) {
            jsonSuccess = false;
            displayError("Error getting messages");
        }
    });
    areThereNewMessagesP();
}


function areThereNewMessagesP()
{
    newMsgUrl = urlPrefixJson + "status/max_msg_id";
    console.log("Checking new messages (using url " + newMsgUrl + ")");

    $.ajax({
        url: newMsgUrl,
        dataType: 'json',
        success: function(data) {
            jsonSuccess = true;
            displayRefresh();
            var remoteMaxId = data[0]["max_msg_id"];
            console.log("max remote id "+remoteMaxId + ", max current id "+maxCurrentId);
            if (remoteMaxId != maxCurrentId) {
                var oldMaxId = maxCurrentId;
                maxCurrentId = remoteMaxId;
                displayConversations(oldMaxId);
            }
        },
        error: function(data) {
            jsonSuccess = false;
            displayError("Error getting messages");
        }
    });
    timerId = window.setTimeout(areThereNewMessagesP, 10000);
}


function gotoConversation(conv_id)
{
    if (timerId > 0) {
        window.clearTimeout(timerId);
        timerId = window.setTimeout(areThereNewMessagesP, 10000);
    }
    currentConversation = parseInt(conv_id.substring(5));
    console.log("Going to conversation " + currentConversation);
    clearAndDisplayConversations();
}


function updateMessagesError()
{
    displayError(textStatus);
}


function updateMessages(data)
{
    $("#messages").append(data);
    console.log("Updating links (conversation=" + currentConversation + ")");
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
    window.setTimeout(postUpdateMessages, 100);
}


/*
 * Perform various tasks after new messages have been received (e.g. scrolling).
 */
function postUpdateMessages()
{
    $("#messages").scrollTop($("#messages").prop("scrollHeight") - $("#messages").height());
}


function displayMessages(responseText, textStatus, oldMaxId)
{
    console.log("displayMessages(" + oldMaxId + ")");
    var imUrl;
    if (textStatus == "success") {
        if (currentConversation == 0) {
            imUrl = urlPrefix + "messages/all";
            if (oldMaxId >= 0) {
                imUrl = imUrl + "/start_from/" + oldMaxId;
            }
        } else {
            imUrl = urlPrefix + "conversations/" + currentConversation;
            if (oldMaxId >= 0) {
                imUrl = imUrl + "/start_from/" + oldMaxId;
            }
        }
        console.log(imUrl);
        $.get(imUrl, function (data) { updateMessages(data); }).fail(updateMessagesError);
    } else {
        displayError(textStatus);
    }
}


function clearAndDisplayConversations()
{
    $("#messages").text("");
    displayConversations(0);
}


function displayConversations(oldMaxId)
{
    console.log("displayConversations(" + oldMaxId + ")");
    // clear old conversation list
    $("#conversations").text("");

    // conversations window
    var conversationsUrl = urlPrefix + "conversations/all";
    $("#conversations").load(conversationsUrl,
                             function (responseText, textStatus)
                             {
                                 displayMessages(responseText, textStatus, oldMaxId);
                             });
}


function sendMessageResult(data)
{
    $("#send_msg_text").val("");
    // wait 1 sec. for the message to be processed
    window.setTimeout(areThereNewMessagesP, 1000);
}


function sendMessageToPurple()
{
    if (timerId > 0) {
        window.clearTimeout(timerId);
        timerId = window.setTimeout(areThereNewMessagesP, 10000);
    }
    postUrl = urlPrefix + "conversations/" + currentConversation;
    message = $("#send_msg_text").val();
    console.log("Sending message " + message + " to address " + postUrl);
    $.post(postUrl, message, function(data, textStatus) {
        console.log(textStatus);
        sendMessageResult(data);
    });
}


areThereNewMessagesP();
