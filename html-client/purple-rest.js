var currentConversation = {};
currentConversation.id = 0;
currentConversation.name = "";

var urlPrefixHtml = "rest/v1/html/";
var urlPrefixJson = "rest/v1/json/";
var timerId = 0;
var maxCurrentId = -1;
var maxCurrentMyMsgId = -1; // max id for my-messages
var jsonSuccess = false;
var dialogBoxMenuActive = false;
var innerContentText = "";

// :fixme: put runtime values in a data structure
var autoRefresh = true;
var autoRefreshInterval = 10000;

var windowIsVisible = false;
var windowWasVisible = false;

var textAreaOrigSize = "";

$('document').ready(function() { textAreaOrigSize = $("#send-msg-text").css("height"); } );

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


/**
 * Returns a string containing the HTML representation of a button in the center menu
 */
function buttonMenuHtmlStr(text, jsText, extraStyle)
{
    var str = '<div class="purple-button-menu-item" onclick="' + jsText + '"';
    if (extraStyle) {
        str = str + ' style="' + extraStyle + '"';
    }
    str = str + '>' + text + '</div>';
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


function enableRefreshTimer(updateDisplay = false)
{
    if (autoRefresh) {
        disableRefreshTimer(); // disable old timer if it exists
        if (windowIsVisible) {
            autoRefreshInterval = 10000;
        } else {
            autoRefreshInterval = 90000;
        }
        timerId = window.setTimeout(areThereNewMessagesP, autoRefreshInterval);
        if (windowIsVisible != windowWasVisible || updateDisplay) {
            var refreshIntervalHr = autoRefreshInterval / 1000;
            $("#status-bar-1").text("Auto-refresh: ON (" + refreshIntervalHr.toString() + "s)");
        }
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
    maxCurrentMyMsgId = -1;
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


function updateVisibility()
{
    windowWasVisible = windowIsVisible;
    windowIsVisible = document.hasFocus();
}


function areThereNewMessagesP()
{
    newMsgUrl = urlPrefixJson + "status/max_msg_id";
    console.log("Checking new messages (using url " + newMsgUrl + ")");
    updateVisibility();

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
                /*
                // display notification only on automatic update
                if (maxCurrentId != -1 && !windowIsVisible) {
                    notifyNewMessage("New chat messages !");
                }
                */
                maxCurrentId = remoteMaxId;
                displayConversations(oldMaxId);
            }
        },
        error: function(data) {
            jsonSuccess = false;
            displayError("Error getting messages");
        }
    });

    if (jsonSuccess) {
        // check if there are new messages for me (my-messages)
        newMyMsgUrl = urlPrefixJson + "status/max_my_msg_id";
        console.log("Checking new 'my messages' (using url " + newMyMsgUrl + ")");
        $.ajax({
            url: newMyMsgUrl,
            dataType: 'json',
            success: function(data) {
                jsonSuccess = true;
                var remoteMaxMyMsgId = data[0]["max_my_msg_id"];
                console.log("max remote my msg id "+remoteMaxMyMsgId + ", max current my msg id " + maxCurrentMyMsgId);
                if (remoteMaxMyMsgId != maxCurrentMyMsgId) {
                    var oldMaxId = maxCurrentMyMsgId;
                    // display notification only on automatic update
                    if (maxCurrentMyMsgId != -1 && !windowIsVisible) {
                        notifyNewMessage("New chat messages !");
                    }
                    maxCurrentMyMsgId = remoteMaxMyMsgId;
                }
            },
            error: function(data) {
                jsonSuccess = false;
                displayError("Error getting my messages");
            }
        });
    }

    enableRefreshTimer();
}


function updateMessagesError()
{
    displayError(textStatus);
}


function hideSendMsg()
{
    $("#send-msg-text").hide()
    $("#send-msg-button").hide()
}


function updateMessages(data)
{
    $("#messages").append(data);

    if (currentConversation.id > 0) {
        // enable input
        $("#send-msg-text").show()
        $("#send-msg-button").show()
    } else {
        // disable input
        hideSendMsg();
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
        '<td style="width: 10%; text-align: right;">' +
          //buttonHtmlStr("Menu", "dialogBoxMenuConvMenu();") +
          buttonHtmlStr("&nbsp;💬&nbsp;", "dialogBoxMenuConvMenu();") +
        "</td>" +
        '<td style="width: 80%; text-align: center;">' +
          convCurrentTitleHtmlStr() +
        "</td>" +
        '<td style="width: 10%; text-align: left;">' +
        //buttonHtmlStr("Switch", "dialogBoxMenuSwitchToConversations();") +
        // ⇿
        //buttonHtmlStr("⬌", "dialogBoxMenuSwitchToConversations();") +
        // « »
        buttonHtmlStr("⇠⇢", "dialogBoxMenuSwitchToConversations();") +
        "</td>" +
        '</tr></table>';

    $("#conversations").html(conversationsLine);
    displayMessages(null, "success", oldMaxId);
}


function textAreaClicked()
{
    if ($("#send-msg-text").css("height") == textAreaOrigSize) {
        textAreaMakeBig();
    }
}


function textAreaMakeBig()
{
    $("#send-msg-text").css("height", "4rem");
    textAreaIsSmall = false;
}


function textAreaMakeSmall()
{
    $("#send-msg-text").css("height", textAreaOrigSize);
    textAreaIsSmall = true;
}


function sendMessageResult(data)
{
    $("#send-msg-text").val("");
    areThereNewMessagesP();
}


function sendMessageToPurple()
{
    disableRefreshTimer();
    var postUrl = urlPrefixHtml + "conv-messages/" + currentConversation.id;
    message = $("#send-msg-text").val();
    console.log("Sending message " + message + " to address " + postUrl);
    textAreaMakeSmall();
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
    return buttonHtmlStr("⬅&nbsp;&nbsp;&nbsp;Back to chat", "dialogBoxMenuExit();",
                     "margin-bottom: 2em; margin-top: 1em;");
}


function prepareForCenterMenu()
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
    prepareForCenterMenu();
    // add menu options
    var menuText = '';
    menuText += '<div class="center-menu">';
    for (var i = 0; i < options.length; i++) {
        menuText += buttonMenuHtmlStr(options[i][0], options[i][1]);
        //menuText += '<br/>';
    }
    menuText += '<br/></div>';
    $("#inner-content").html(prefix + dialogBoxMenuBackButtonStr() + menuText);
}


function dialogBoxMenu()
{
    var autoRefreshText = "🔃 ";
    autoRefreshText += (autoRefresh ? 'Disable auto refresh' : 'Enable auto refresh');
    menuOptions = [ [ autoRefreshText, "dialogBoxMenuAutoRefresh();" ],
                    [ "👤 Online buddies", "dialogBoxMenuBuddies(true);" ],
                    [ "👥 All buddies", "dialogBoxMenuBuddies();" ],
                    [ "⚙ Accounts", "dialogBoxMenuAccounts();" ],
                    [ "🗈 Show status", "dialogBoxMenuGetStatus();" ],
                    [ "🖦 Reset idle time", "dialogBoxMenuSetStatus('reset-idle');" ],
                    [ "🗪 Set status 'Available'", "dialogBoxMenuSetStatus('available');" ],
                    [ "⏳ Set status 'Away'", "dialogBoxMenuSetStatus('away');" ],
                    [ "∅ Set status 'Invisible'", "dialogBoxMenuSetStatus('invisible');" ],
                  ];
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
    var statusUrl = urlPrefixJson + "accounts/all/status";
    // :fixme: add another error function
    $.get(statusUrl, function (data) { showAccountsStatus(data); }).fail(updateMessagesError);
}


function dialogBoxMenuSetStatus(newStatus)
{
    var putStatusUrl = urlPrefixJson;
    if (newStatus == "reset-idle") {
        putStatusUrl += "reset-idle/";
    } else {
        putStatusUrl += "accounts/all/status/" + newStatus;
    }
    $.ajax({
        url: putStatusUrl,
        type: 'put',
        dataType: 'json',
        success: function(data) {
            showAccountsStatus(data);
        },
        error: function(data) {
            displayError("Error changing status");
        }
    });
}


// Display a message box containing the result of an operation (OK or something similar).
// data should have a field named 'status', otherwise 'OK' is used.
function showOperationResult(data)
{
    // :fixme: check if data is an array and has that property
    var statusText = "";
    if (data) {
        statusText = data[0].status;
    } else {
        statusText = "OK";
    }
    $("#inner-content").html(dialogBoxMenuBackButtonStr() +
                             messageBoxText("Status: " + statusText, "Status"));
}


// Display the active status for all the accounts
function showAccountsStatus(data)
{
    var statusText = "";
    if (data) {
        for (var i = 0; i < data.length; i++) {
            var acc = Object.keys(data[i])[0];
            var text = acc + '&nbsp;:&nbsp;' + data[i][acc];
            statusText += '<div class="account"><span class="account">'
                + text + '</span></div><hr class="account"/>\n';
            $("#inner-content").html(dialogBoxMenuBackButtonStr() +
                                     messageBoxText(statusText, "Accounts status"));
        }
    } else {
        displayError("Error getting accounts status");
    }
}


// Display all statuses (with details) for an account
function showAccountStatuses(data)
{
    var statusText = "";
    if (data) {
        var statuses = data[0]["Statuses"];
        for (var i = 0; i < statuses.length; i++) {
            statusText += statuses[i] + '</br>\n';
        }
        $("#inner-content").html(dialogBoxMenuBackButtonStr() +
                                 messageBoxText(statusText, "Account statuses"));
    } else {
        displayError("Error getting accounts status");
    }
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
    htmlStr += '</br>';
    $("#inner-content").html(htmlStr);
}


function dialogBoxMenuAccountsDisplay(accounts)
{
    var htmlStr = dialogBoxMenuBackButtonStr();
    var accountsStr = "";
    // :fixme: check if data is valid
    for (var i = 0; i < accounts.length; i++) {
        accountsStr += accountHtmlStr(accounts[i]);
    }
    htmlStrFull = htmlStr + messageBoxText(accountsStr, "Accounts");
    $("#inner-content").html(htmlStrFull);
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
        enableRefreshTimer(true);
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

/*
function displayConversationButton(conv)
{
    var buttonText = buttonMenuHtmlStr(conv.name, jsFuncText);
    $("#inner-content").append(buttonText);
}
*/

/**
 * Display the list of conversations (and the 'back' button) into the main dialog window.
 */
function showConversationsList(data)
{
    var menuOptions = [ ];
    var menuText = '';

    var conversations = data;
    var jsFuncText = '';
    for (var i = 0; i < conversations.length; i++) {
        jsFuncText = 'dialogBoxMenuGotoConversation(' + conversations[i].id + ', \'' + conversations[i].name + '\');'
        menuOptions.push( [ conversations[i].name, jsFuncText] );
    }
    /*$("#inner-content").append('<br/></div>');*/
    displayCenterMenu(menuOptions, menuText);
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
        '<td style="width: 40%; text-align: center;">' +
        buttonHtmlStr("All messages", "allMsgs();") + '</td>' +
        '<td style="width: 40%; text-align: center;">' +
        buttonHtmlStr("Clear history", "clearHistory();") + '</td>' +
        '<td style="width: 10%; text-align: right;">' +
        //buttonHtmlStr("⭮", "clearAndDisplayConversations();") +
        buttonHtmlStr("⤿", "clearAndDisplayConversations();") +
        '</td></tr></table>';
    $("#menu").html(mainMenuText);
}


/**
 * Greys out the main menu buttons.
 */
function disableMainMenu()
{
    $("#menu span").prop("onclick", null).off("click");
    $("#menu span").attr("class", "purple-button-disabled");
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
    var menuText = '';
    if (currentConversation.id != 0)  {
        menuOptions.push([ "Close conversation", "convClose();" ]);
        menuText = "<br/><div>" + convCurrentTitleHtmlStr() + "</div><br/>";
    } else {
        menuText = "<br/><div><b>No options for 'All messages'<br/>Select a specific chat !</b></div><br/>";
    }
    displayCenterMenu(menuOptions, menuText);
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
                showOperationResult(data);
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
    var accountName;
    if (account.alias != 'None') {
        accountName = account.alias;
    } else {
        accountName = account.username;
    }
    // :fixme: escape accountName
    if (accountName.charAt(accountName.length-1) == '/') {
        accountName = accountName.substring(0, accountName.length-1);
    }
    var funcStatuses = 'onAccountStatusesClick(\'' + accountName + '\');';
    htmlStr = '<div class="account"><span class="account-name">'
        + accountName + '&nbsp;:&nbsp;</span> ';
    htmlStr += smallButtonHtmlStr('All statuses', funcStatuses);
    htmlStr += '</div><hr class="account"/>\n';
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


function onAccountStatusesClick(name)
{
    console.log("Getting statuses for " + name);
    var getStatusesUrl = urlPrefixJson + "accounts/" + name + "/statuses";
    $.get(getStatusesUrl,
          function (data) { showAccountStatuses(data); }).fail( function() { displayError("Error getting statuses"); });
}


function notifyNewMessage(msg)
{
  var notifOptions = { tag : 'purple-rest-new-chat-msg' };
  // Let's check if the browser supports notifications
  if (!("Notification" in window)) {
    alert("This browser does not support desktop notification");
  }

  // Let's check whether notification permissions have already been granted
  else if (Notification.permission === "granted") {
    // If it's okay let's create a notification
    var notification = new Notification(msg, notifOptions);
  }

  // Otherwise, we need to ask the user for permission
  else if (Notification.permission !== "denied") {
    Notification.requestPermission().then(function (permission) {
      // If the user accepts, let's create a notification
      if (permission === "granted") {
        var notification = new Notification(msg, notifOptions);
      }
    });
  }

  // At last, if the user has denied notifications, and you
  // want to be respectful there is no need to bother them any more.
}


function messageBoxText(message, title = "")
{
    var messageBoxText = '<div class="message-box">';
    if (title) {
        messageBoxText += '<div class="message-box-title">';
        messageBoxText += title;
        messageBoxText += '</div>\n';
    }
    messageBoxText += '<div class="message-box-text">';
    messageBoxText += message;
    messageBoxText += '</div></div><br/>\n';
    return messageBoxText;
}

showMainMenu();
areThereNewMessagesP();
