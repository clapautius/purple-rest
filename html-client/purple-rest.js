var currentConversation = {};
currentConversation.id = 0;
currentConversation.name = "";

var urlPrefixHtml = "rest/v1/html/";
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


function enableRefreshTimer()
{
    if (autoRefresh) {
        disableRefreshTimer(); // disable old timer if it exists
        timerId = window.setTimeout(areThereNewMessagesP, 10000);
    }
}


function disableRefreshTimer()
{
    if (timerId > 0) {
        window.clearTimeout(timerId);
        timerId = 0;
    }
}


function clearHistory()
{
    if (!preMenuCommand('Clear history')) {
        return false;
    }
    disableRefreshTimer();
    $("#messages").text("");
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
    enableRefreshTimer();
}


function updateMessagesError()
{
    displayError(textStatus);
}


function updateMessages(data)
{
    $("#messages").append(data);

    if (currentConversation.id > 0) {
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
        if (currentConversation.id == 0) {
            imUrl = urlPrefixHtml + "messages/all";
            if (oldMaxId >= 0) {
                imUrl = imUrl + "/start_from/" + oldMaxId;
            }
        } else {
            imUrl = urlPrefixHtml + "conv-messages/" + currentConversation.id;
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
    if (!preMenuCommand()) {
        return false;
    }

    $("#messages").text("");
    maxCurrentId = -1;
    areThereNewMessagesP();
}


function displayConversations(oldMaxId)
{
    console.log("displayConversations(" + oldMaxId + ")");

    // conversations window
    // first button is always 'all msgs.'
    // :fixme: get rid of this table
    var conversationsLine = '<table style="text-align: center;"><tr><td style="width: 20%;"><span style="width: 20%;" class="menu" onclick="currentConversation.id=0; clearAndDisplayConversations();">All msgs.</span></td>';

    conversationsLine = conversationsLine +
        '<td><span style="text-decoration: underline; font-weight: bold;">' +
        (currentConversation.id > 0 ? currentConversation.name : 'All msgs.') +
        '</span></td>';

    conversationsLine = conversationsLine +
        '<td style="width: 20%;"><span style="width: 20%;" class="menu" \
onclick="mainMenuSwitchToConversations();">Switch to conv.\
</span></td>';

    $("#conversations").html(conversationsLine);
    displayMessages(null, "success", oldMaxId);
}


function sendMessageResult(data)
{
    $("#send_msg_text").val("");
    // wait 1 sec. for the message to be processed
    window.setTimeout(areThereNewMessagesP, 1000);
}


function sendMessageToPurple()
{
    disableRefreshTimer();
    var postUrl = urlPrefixHtml + "conv-messages/" + currentConversation.id;
    message = $("#send_msg_text").val();
    console.log("Sending message " + message + " to address " + postUrl);
    $.post(postUrl, message, function(data, textStatus) {
        console.log(textStatus);
        sendMessageResult(data);
    });
}


function mainMenuBackButton()
{
    var buttonText = '<br/><span class="menu" onclick="mainMenuExit();" style="margin-top: 2em;">Back to chat</span><br/><br/>';
    $("#inner-content").append(buttonText);
}


function prepareForMainMenu()
{
    // display a menu inside 'inner-content' div
    // put original 'inner-content' text in global var inner-content-text
    // :fixme: is there a better option?
    innerContentText = $("#inner-content").html();
    mainMenuActive = true;
}


function mainMenu()
{
    // don't do anything if the menu is already active
    if (mainMenuActive) {
        return;
    }
    prepareForMainMenu();
    // add menu options
    menuText = '<br/><span class="menu" onclick="mainMenuBuddies();">Buddies</span><br/>';
    menuText = menuText + '<br/><span class="menu" onclick="mainMenuAutoRefresh();">';
    if (autoRefresh) {
        menuText = menuText + 'Disable auto refresh';
    } else {
        menuText = menuText + 'Enable auto refresh';
    }
    menuText = menuText + '</span><br/>';

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
    var buddiesUrl = urlPrefixHtml + "buddies/all";
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
        disableRefreshTimer();
        autoRefresh = false;
        $("#status-bar-1").text("Auto-refresh: OFF");
    } else {
        autoRefresh = true;
        $("#status-bar-1").text("Auto-refresh: ON");
        enableRefreshTimer();
    }
    $("#inner-content").html('<div class="info-msg"><span class="info-msg">Done</span></div>');
    mainMenuBackButton();
}


/**
 * Performs first step for switching the conversations - gets the conversation list.
 * On success, goes to 'showConversationsList()'.
 */
function mainMenuSwitchToConversations()
{
    preMenuCommand("switch-to-conv");
    var conversationsUrl = urlPrefixJson + "conversations/all";
    // :fixme: add another error function
    $.get(conversationsUrl, function (data) { showConversationsList(data); }).fail(updateMessagesError);
}


/**
 * Clear the main dialog window and displays the specified conversation.
 */
function mainMenuGotoConversation(conv_id, conv_name)
{
    currentConversation.id = conv_id;
    currentConversation.name = conv_name;
    console.log("Going to conversation " + currentConversation.id);
    mainMenuExit();
}


function displayConversationButton(conv)
{
    var buttonText = '<br/><span class="menu" onclick="mainMenuGotoConversation(' +
        conv.id + ', \'' + conv.name + '\');">' + conv.name + '</span><br/>';
    $("#inner-content").append(buttonText);
}


/**
 * Display the list of conversations (and the 'back' button) into the main dialog window.
 */
function showConversationsList(data)
{
    prepareForMainMenu();
    $("#inner-content").text("");
    var conversations = data;
    for (var i = 0; i < conversations.length; i++) {
        displayConversationButton(conversations[i]);
    }
    mainMenuBackButton();
}


areThereNewMessagesP();
