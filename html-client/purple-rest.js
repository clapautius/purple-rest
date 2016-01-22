var currentConversation = {};
currentConversation.id = 0;
currentConversation.name = "";

var urlPrefixHtml = "rest/v1/html/";
var urlPrefixJson = "rest/v1/json/";
var timerId = 0;
var maxCurrentId = -1;
var jsonSuccess = false;
var dialogBoxMenuActive = false;
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
        $("#status-bar-2").html("Updated: " + formatted);
    }
}


/**
 * Returns a string containing the HTML representation of a button.
 */
function buttonStr(text, jsText, extraStyle)
{
    var str = '<span class="purple-button" onclick="' + jsText + '"';
    if (extraStyle) {
        str = str + ' style="' + extraStyle + '"';
    }
    str = str + '>' + text + '</span>';
    return str;
}


// function called before any command (with the name of the command as parameter
// can return false, in this case the command is not executed
function preMenuCommand(cmd)
{
    if (dialogBoxMenuActive) {
        if (cmd == 'All msgs.' || cmd == 'Clear history') {
            return false;
        }
    }
    return true;
}


/*
  :fixme: - deprecated?
*/
function desktopLayout()
{
    if (!preMenuCommand('Desktop layout')) {
        return;
    }

    $("#content").css("width", "50em");
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


function allMsgs()
{
    if (!preMenuCommand('All msgs.')) {
        return false;
    }
    currentConversation.id=0;
    clearAndDisplayConversations();
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
    // :fixme: get rid of this table
    var conversationsLine = '<table cellpadding="0" style="text-align: center; width: 100%;"><tr>' +
        '<td style="width: 50%; text-align: left;">' +
        convCurrentTitleAsHtml() +
        '</td>';

    conversationsLine = conversationsLine + '<td style="width: 50%; text-align: right;">' +
        buttonStr("Conv. menu", "dialogBoxMenuConvMenu();") +
        buttonStr("Switch conv.", "dialogBoxMenuSwitchToConversations();") + '</td></table>';

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

/* DEPRECATED
function dialogBoxMenuBackButton()
{
    var buttonText = buttonStr("Back to chat", "dialogBoxMenuExit();", "margin-bottom: 2em;") + '<br/>';
    $("#inner-content").append(buttonText);
}
*/

function dialogBoxMenuBackButtonStr()
{
    return buttonStr("Back to chat", "dialogBoxMenuExit();",
                     "margin-bottom: 2em; margin-top: 1em;");
}


function prepareForMainMenu()
{
    // display a menu inside 'inner-content' div
    // put original 'inner-content' text in global var inner-content-text
    // :fixme: is there a better option?
    innerContentText = $("#inner-content").html();

    disableMainMenu();
    dialogBoxMenuActive = true;
}


function displayCenterMenu(options, prefix = "")
{
    // don't do anything if the menu is already active
    if (dialogBoxMenuActive) {
        return;
    }
    prepareForMainMenu();
    // add menu options
    var menuText = '<br/>';
    for (var i = 0; i < options.length; i++) {
        menuText += buttonStr(options[i][0], options[i][1]);
        menuText += '<br/><br/>';
    }
    $("#inner-content").html(prefix + dialogBoxMenuBackButtonStr() + menuText);
}


function dialogBoxMenu()
{
    var autoRefreshText = (autoRefresh ? 'Disable auto refresh' : 'Enable auto refresh');
    menuOptions = [ [ autoRefreshText, "dialogBoxMenuAutoRefresh();" ],
                    [ "Show status", "dialogBoxMenuGetStatus();" ],
                    [ "Online buddies", "dialogBoxMenuBuddies(true);" ],
                    [ "All buddies", "dialogBoxMenuBuddies();" ],
                    [ "Desktop layout", "desktopLayout();" ] ];
    displayCenterMenu(menuOptions);
}


function dialogBoxMenuExit()
{
    // enable other ocmmand
    $("#inner-content").html(innerContentText);
    dialogBoxMenuActive = false;

    enableMainMenu();
    clearAndDisplayConversations();
}


function dialogBoxMenuBuddies(onlineOnly = false)
{
    // get buddies
    var buddiesUrl = '';
    if (onlineOnly) {
        buddiesUrl = urlPrefixHtml + "buddies/online";
    } else {
        buddiesUrl = urlPrefixHtml + "buddies/all";
    }
    $("#inner-content").load(buddiesUrl,
                             function (responseText, textStatus) {
                                 dialogBoxMenuBuddiesDisplay(responseText, textStatus);
                             });
}


function dialogBoxMenuGetStatus()
{
    var statusUrl = urlPrefixJson + "status/account-status";
    // :fixme: add another error function
    $.get(statusUrl, function (data) { showStatus(data); }).fail(updateMessagesError);
}


function showStatus(data)
{
    // :fixme: check if data is an array and has that property
    var statusText = "";
    if (data) {
        statusText = data[0].status;
    } else {
        statusText = "OK";
    }
    $("#inner-content").html(dialogBoxMenuBackButtonStr() +
                             '<div class="info-msg"><span class="info-msg">Status: ' +
                              statusText +
                             '</span></div>');
}


function dialogBoxMenuBuddiesDisplay()
{
    $("#inner-content").prepend(dialogBoxMenuBackButtonStr());
}


function dialogBoxMenuAutoRefresh()
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
    $("#inner-content").html(dialogBoxMenuBackButtonStr() +
                             '<div class="info-msg"><span class="info-msg">Done</span></div>');
}


/**
 * Performs first step for switching the conversations - gets the conversation list.
 * On success, goes to 'showConversationsList()'.
 *
 * :fixme: this is not from dialogBoxMenu - to be renamed
 */
function dialogBoxMenuSwitchToConversations()
{
    preMenuCommand("switch-to-conv");
    var conversationsUrl = urlPrefixJson + "conversations/all";
    // :fixme: add another error function
    $.get(conversationsUrl, function (data) { showConversationsList(data); }).fail(updateMessagesError);
}


/**
 * Clear the main dialog window and displays the specified conversation.
 */
function dialogBoxMenuGotoConversation(conv_id, conv_name)
{
    currentConversation.id = conv_id;
    currentConversation.name = conv_name;
    console.log("Going to conversation " + currentConversation.id);
    dialogBoxMenuExit();
}


function displayConversationButton(conv)
{
    var jsFuncText = 'dialogBoxMenuGotoConversation(' + conv.id + ', \'' + conv.name + '\');'
    var buttonText = '<br/>' + buttonStr(conv.name, jsFuncText) + '<br/>';
    $("#inner-content").append(buttonText);
}


/**
 * Display the list of conversations (and the 'back' button) into the main dialog window.
 */
function showConversationsList(data)
{
    prepareForMainMenu();
    $("#inner-content").text("");
    $("#inner-content").append(dialogBoxMenuBackButtonStr());
    var conversations = data;
    for (var i = 0; i < conversations.length; i++) {
        displayConversationButton(conversations[i]);
    }
    $("#inner-content").append("<br/>");
}


/**
 * Add main menu items.
 */
function showMainMenu()
{
    mainMenuText = buttonStr("&#9776;", "dialogBoxMenu();") +
        buttonStr("All msgs.", "allMsgs();") +
        buttonStr("Clear history", "clearHistory();");
    $("#menu").html(mainMenuText);
}


/**
 * Greys out the main menu buttons.
 */
function disableMainMenu()
{
    $("#menu > span").prop("onclick", null).off("click");
    $("#menu > span").attr("class", "purple-button-disabled");
}


/**
 * Enable the main menu buttons.
 */
function enableMainMenu()
{
    showMainMenu();
}



/**
 * :fixme: this is not from dialogBoxMenu - to be renamed
 */
function dialogBoxMenuConvMenu()
{
    menuOptions = [ ];
    if (currentConversation.id != 0)  {
        menuOptions.push([ "Close conversation", "convClose();" ]);
    }
    displayCenterMenu(menuOptions,
                      "<br/><div>" + convCurrentTitleAsHtml() + "</div><br/>");
}


/**
 * Closes the current conversation (it does nothing if the current conv. is 'All msgs.').
 */
function convClose()
{
    if (currentConversation.id != 0) {
        var delConvMsgCmd = urlPrefixJson + "conversations/" + currentConversation.id;
        $.ajax({
            url: delConvMsgCmd,
            type: 'delete',
            success: function(data) {
                currentConversation.id = 0;
                showStatus(data);
            },
            error: function(data) {
                displayError("Error closing conversation");
            }
        });
    }
}


function convCurrentTitleAsHtml()
{
    return '<span class="conv-title">➤&nbsp;' +
        (currentConversation.id > 0 ? currentConversation.name : 'All msgs.') +
        '</span>';
}


showMainMenu();
areThereNewMessagesP();
