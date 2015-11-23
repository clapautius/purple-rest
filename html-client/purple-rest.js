var currentConversation = 0;
var urlPrefix = "rest/v1/html/";
var urlPrefixJson = "rest/v1/json/";
var timerId = 0;
var maxCurrentId = -1;
var jsonSuccess = false;
var mainMenuActive = false;
var innerContentText = "";

// :fixme: put runtime values in a data structure
var autoRefresh = true;


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


// function called before any command (with the name of the command as parameter
// can return false, in this case the command is not executed
function preMenuCommand(cmd)
{
    if (mainMenuActive) {
        if (cmd == 'Mobile layout' || cmd == 'All msgs.' ||
            cmd == 'Clear history') {
            return false;
        }
    }
    return true;
}


function mobileLayout()
{
    if (!preMenuCommand('Mobile layout')) {
        return;
    }

    $("#content").css("width", "100%");
}


function enableTimerId()
{
    if (autoRefresh) {
        timerId = window.setTimeout(areThereNewMessagesP, 10000);
    }
}


function clearHistory()
{
    if (!preMenuCommand('Clear history')) {
        return false;
    }

    $("#messages").text("");
    if (timerId > 0) {
        window.clearTimeout(timerId);
        enableTimerId();
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
    enableTimerId();
}


function gotoConversation(conv_id)
{
    if (timerId > 0) {
        window.clearTimeout(timerId);
        enableTimerId();
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
            imUrl = urlPrefix + "conv-messages/" + currentConversation;
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
    if (!preMenuCommand('All msgs.')) {
        return false;
    }

    $("#messages").text("");
    maxCurrentId = 0;
    areThereNewMessagesP();
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
        enableTimerId();
    }
    postUrl = urlPrefix + "conv-messages/" + currentConversation;
    message = $("#send_msg_text").val();
    console.log("Sending message " + message + " to address " + postUrl);
    $.post(postUrl, message, function(data, textStatus) {
        console.log(textStatus);
        sendMessageResult(data);
    });
}


function mainMenuBackButton()
{
    buttonText = '<span class="menu" onclick="mainMenuExit();" style="margin-top: 2em;">Back to chat</span><br/><br/>';
    $("#inner-content").append(buttonText);
}


function mainMenu()
{
    // don't do anything if the menu is already active
    if (mainMenuActive) {
        return;
    }
    // display a menu inside 'inner-content' div
    // put original 'inner-content' text in global var inner-content-text
    // :fixme: is there a better option?
    innerContentText = $("#inner-content").html();
    mainMenuActive = true;
    // add menu options
    menuText = '<br/><span class="menu" onclick="mainMenuBuddies();">Buddies</span><br/>';
    menuText = menuText + '<br/><span class="menu" onclick="mainMenuAutoRefresh();">';
    if (autoRefresh) {
        menuText = menuText + 'Disable auto refresh';
    } else {
        menuText = menuText + 'Enable auto refresh';
    }
    menuText = menuText + '</span><br/>';

    menuText = menuText + '<br/>';
    $("#inner-content").html(menuText);
    mainMenuBackButton();
}


function mainMenuExit()
{
    // enable other ocmmand
    $("#inner-content").html(innerContentText);
    mainMenuActive = false;
    clearAndDisplayConversations();
}


function mainMenuBuddies()
{
    // get buddies
    var buddiesUrl = urlPrefix + "buddies/all";
    $("#inner-content").load(buddiesUrl,
                             function (responseText, textStatus) {
                                 mainMenuBuddiesDisplay(responseText, textStatus);
                             });
}


function mainMenuBuddiesDisplay()
{
    mainMenuBackButton();
}


function mainMenuAutoRefresh()
{
    preMenuCommand("auto-refresh");
    if (autoRefresh) {
        if (timerId > 0) {
            window.clearTimeout(timerId);
        }
        autoRefresh = false;
        $("#status-bar-1").text("Auto-refresh: OFF");
    } else {
        autoRefresh = true;
        $("#status-bar-1").text("Auto-refresh: ON");
        if (timerId > 0) {
            window.clearTimeout(timerId);
        }
        enableTimerId();
    }
    $("#inner-content").html('<div class="info-msg"><span class="info-msg">Done</span></div>');
    mainMenuBackButton();
}


areThereNewMessagesP();
