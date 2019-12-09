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
function buttonHtmlStr(text, jsText, extraStyle)
{
    var str = '<span class="purple-button" onclick="' + jsText + '"';
    if (extraStyle) {
        str = str + ' style="' + extraStyle + '"';
    }
    str = str + '>' + text + '</span>';
    return str;
}


/**
 * Returns a string containing the HTML representation of a button.
 */
function smallButtonHtmlStr(text, jsText, extraStyle)
{
    var str = '<span class="purple-small-button" onclick="' + jsText + '"';
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
                notifyNewMessage("New chat messages !");
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
    var conversationsLine = '<table cellpadding="0" style="text-align: center; width: 100%;">' + '<tr>' +
        '<td style="width: 20%; text-align: middle;">' +
          //buttonHtmlStr("Menu", "dialogBoxMenuConvMenu();") +
          buttonHtmlStr("💬", "dialogBoxMenuConvMenu();") +
        "</td>" +
        '<td style="width: 60%; text-align: middle;">' +
          convCurrentTitleHtmlStr() +
        "</td>" +
        '<td style="width: 20%; text-align: middle;">' +
          //buttonHtmlStr("Switch", "dialogBoxMenuSwitchToConversations();") +
          // ⇿
          buttonHtmlStr("⬌", "dialogBoxMenuSwitchToConversations();") +
        "</td>" +
        '</tr></table>';

    $("#conversations").html(conversationsLine);
    displayMessages(null, "success", oldMaxId);
}


function sendMessageResult(data)
{
    $("#send_msg_text").val("");
    areThereNewMessagesP();
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
    var buttonText = buttonHtmlStr("Back to chat", "dialogBoxMenuExit();", "margin-bottom: 2em;") + '<br/>';
    $("#inner-content").append(buttonText);
}
*/

function dialogBoxMenuBackButtonStr()
{
    return buttonHtmlStr("Back to chat", "dialogBoxMenuExit();",
                     "margin-bottom: 2em; margin-top: 1em;");
}


function prepareForMainMenu()
{
    // display a menu inside 'inner-content' div
    // put original 'inner-content' text in global var inner-content-text
    // :fixme: is there a better option?
    innerContentText = $("#inner-content").html();

    disableRefreshTimer();
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
        menuText += buttonHtmlStr(options[i][0], options[i][1]);
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
                    [ "Accounts", "dialogBoxMenuAccounts();" ],
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
        buddiesUrl = urlPrefixJson + "buddies/online";
    } else {
        buddiesUrl = urlPrefixJson + "buddies/all";
    }

    $.get(buddiesUrl, function (data) { dialogBoxMenuBuddiesDisplay(data); }).fail( function() { showError("Error getting buddy list"); });
}


function dialogBoxMenuAccounts()
{
    // get accounts
    var accountsUrl =  urlPrefixJson + "accounts/all";

    $.get(accountsUrl, function (data) { dialogBoxMenuAccountsDisplay(data); }).fail( function() { showError("Error getting accounts list"); });
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


function showError(errMsg)
{
    $("#inner-content").html(dialogBoxMenuBackButtonStr() +
                             '<div class="error-msg"><span class="error-msg">Status: ' +
                              errMsg +
                             '</span></div>');
}


function dialogBoxMenuBuddiesDisplay(buddies)
{
    var htmlStr = dialogBoxMenuBackButtonStr();
    // :fixme: check if data is valid
    for (var i = 0; i < buddies.length; i++) {
        htmlStr += buddyHtmlStr(buddies[i]);
    }
    $("#inner-content").html(htmlStr);
}


function dialogBoxMenuAccountsDisplay(accounts)
{
    var htmlStr = dialogBoxMenuBackButtonStr();
    // :fixme: check if data is valid
    for (var i = 0; i < accounts.length; i++) {
        htmlStr += accountHtmlStr(accounts[i]);
    }
    $("#inner-content").html(htmlStr);
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
    var buttonText = '<br/>' + buttonHtmlStr(conv.name, jsFuncText) + '<br/>';
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
    // :fixme: get rid of this table
    mainMenuText = '<table class="main-menu" cellpadding="0" style="text-align: center; width: 100%;">' + '<tr>' +
        '<td style="width: 10%; text-align: left;">' +
        buttonHtmlStr("&#9776;", "dialogBoxMenu();") + '</td>' +
        '<td style="width: 40%; text-align: middle;">' +
        buttonHtmlStr("All messages", "allMsgs();") + '</td>' +
        '<td style="width: 40%; text-align: middle;">' +
        buttonHtmlStr("Clear history", "clearHistory();") + '</td>' +
        '<td style="width: 10%; text-align: right;">' +
        buttonHtmlStr("⭮", "displayError('Not ready yet');") + '</td></tr></table>';
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
                      "<br/><div>" + convCurrentTitleHtmlStr() + "</div><br/>");
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


function convCurrentTitleHtmlStr()
{
    //var prefix = '➤&nbsp;';
    var prefix = '❲&nbsp;';
    var suffix = '&nbsp;❳';
    return '<span class="conv-title">' + prefix +
        (currentConversation.id > 0 ? currentConversation.name : 'All messages') +
        suffix + '</span>';
}


function buddyHtmlStr(buddy)
{
    var htmlStr;
    if (buddy.status == 'online') {
        var funcName = 'onBuddyClick(\'' + buddy.name + '\');';
        htmlStr = '<div class="buddy buddy-online"><span class="buddy-name buddy-online">'
            + buddy.name + '</span>'
            + smallButtonHtmlStr('Chat', funcName)
            + '</div><hr class="buddy"/>\n';
    } else {
        htmlStr = '<div class="buddy buddy-offline"><span class="buddy-name buddy-offline">'
            + buddy.name + '</span></div><hr class="buddy"/>\n';
    }
    return htmlStr;
}


function accountHtmlStr(account)
{
    var htmlStr;
    htmlStr = '<div class="account"><span class="account">'
            + account.alias + '</span></div><hr class="account"/>\n';
    return htmlStr;
}


function onBuddyClick(name)
{
    console.log("Creating new conversation for " + name);
    var putUrl = urlPrefixJson + "conversations/" + name;
    $.ajax({
        url: putUrl,
        type: 'put',
        dataType: 'json',
        success: function(data) {
            currentConversation.id = +data[0].id;
            currentConversation.name = data[0].name;
            dialogBoxMenuExit();
        },
        error: function(data) {
            displayError("Error creating conversation");
        }
    });
}


function notifyNewMessage(msg) {
  // Let's check if the browser supports notifications
  if (!("Notification" in window)) {
    alert("This browser does not support desktop notification");
  }

  // Let's check whether notification permissions have already been granted
  else if (Notification.permission === "granted") {
    // If it's okay let's create a notification
    var notification = new Notification(msg);
  }

  // Otherwise, we need to ask the user for permission
  else if (Notification.permission !== "denied") {
    Notification.requestPermission().then(function (permission) {
      // If the user accepts, let's create a notification
      if (permission === "granted") {
        var notification = new Notification(msg);
      }
    });
  }

  // At last, if the user has denied notifications, and you
  // want to be respectful there is no need to bother them any more.
}

showMainMenu();
areThereNewMessagesP();
