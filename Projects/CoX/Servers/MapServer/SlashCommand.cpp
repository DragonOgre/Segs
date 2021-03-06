/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "SlashCommand.h"

#include "DataHelpers.h"
#include "MapInstance.h"
#include "Settings.h"
#include "GameData/playerdata_definitions.h"
#include "NetStructures/LFG.h"
#include "MapServerData.h"
#include "Logging.h"
#include "NetStructures/Entity.h"
#include "NetStructures/Character.h"

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QRegularExpression>
#include <QtCore/QDebug>

namespace  {
class InfoMessageCmd; // leverage InfoMessageCmd

struct SlashCommand
{
    QStringList m_valid_prefixes;
    QString m_help_text;
    std::function<void(const QString &,MapClientSession &)> m_handler;
    uint32_t m_required_access_level;
};

//bool canAccessCommand(const SlashCommand &cmd, const Entity &e); --> function not defined (yet)
bool canAccessCommand(const SlashCommand &cmd, MapClientSession &src);

// prototypes of all commands
// Access Level 9 Commands (GMs)
void cmdHandler_Script(const QString &cmd, MapClientSession &sess);
void cmdHandler_Dialog(const QString &cmd, MapClientSession &sess);
void cmdHandler_InfoMessage(const QString &cmd, MapClientSession &sess);
void cmdHandler_SmileX(const QString &cmd, MapClientSession &sess);
void cmdHandler_Fly(const QString &cmd, MapClientSession &sess);
void cmdHandler_Falling(const QString &cmd, MapClientSession &sess);
void cmdHandler_Sliding(const QString &cmd, MapClientSession &sess);
void cmdHandler_Jumping(const QString &cmd, MapClientSession &sess);
void cmdHandler_Stunned(const QString &cmd, MapClientSession &sess);
void cmdHandler_Jumppack(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSpeed(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetBackupSpd(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetJumpHeight(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetHP(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetEnd(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetXP(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetDebt(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetInf(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetLevel(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetCombatLevel(const QString &cmd, MapClientSession &sess);
void cmdHandler_UpdateChar(const QString &cmd, MapClientSession &sess);
void cmdHandler_DebugChar(const QString &cmd, MapClientSession &sess);
void cmdHandler_ControlsDisabled(const QString &cmd, MapClientSession &sess);
void cmdHandler_UpdateId(const QString &cmd, MapClientSession &sess);
void cmdHandler_FullUpdate(const QString &cmd, MapClientSession &sess);
void cmdHandler_HasControlId(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetTeam(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSuperGroup(const QString &cmd, MapClientSession &sess);
void cmdHandler_SettingsDump(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamDebug(const QString &cmd, MapClientSession &sess);
void cmdHandler_GUIDebug(const QString &, MapClientSession &sess);
void cmdHandler_SetWindowVisibility(const QString &cmd, MapClientSession &sess);
void cmdHandler_KeybindDebug(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleLogging(const QString &cmd, MapClientSession &sess);
void cmdHandler_FriendsListDebug(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendFloatingNumbers(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleExtraInfo(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleMoveInstantly(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetU1(const QString &cmd, MapClientSession &sess);
// Access Level 2[GM] Commands
void addNpc(const QString &cmd, MapClientSession &sess);
void moveTo(const QString &cmd, MapClientSession &sess);
// Access Level 1 Commands
void cmdHandler_CmdList(const QString &cmd, MapClientSession &sess);
void cmdHandler_AFK(const QString &cmd, MapClientSession &sess);
void cmdHandler_WhoAll(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetTitles(const QString &cmd, MapClientSession &sess);
void cmdHandler_Stuck(const QString &cmd, MapClientSession &sess);
void cmdHandler_LFG(const QString &cmd, MapClientSession &sess);
void cmdHandler_MOTD(const QString &cmd, MapClientSession &sess);
void cmdHandler_Invite(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamAccept(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamDecline(const QString &cmd, MapClientSession &sess);
void cmdHandler_Kick(const QString &cmd, MapClientSession &sess);
void cmdHandler_LeaveTeam(const QString &cmd, MapClientSession &sess);
void cmdHandler_FindMember(const QString &cmd, MapClientSession &sess);
void cmdHandler_MakeLeader(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetAssistTarget(const QString &cmd, MapClientSession &sess);
void cmdHandler_Sidekick(const QString &cmd, MapClientSession &sess);
void cmdHandler_SidekickAccept(const QString &cmd, MapClientSession &sess);
void cmdHandler_SidekickDecline(const QString &cmd, MapClientSession &sess);
void cmdHandler_UnSidekick(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamBuffs(const QString &cmd, MapClientSession &sess);
void cmdHandler_Friend(const QString &cmd, MapClientSession &sess);
void cmdHandler_Unfriend(const QString &cmd, MapClientSession &sess);
void cmdHandler_FriendList(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailHeaders(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailRead(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailSend(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailDelete(const QString &cmd, MapClientSession &sess);

static const SlashCommand g_defined_slash_commands[] = {
    /* Access Level 9 Commands */
    {{"Script"},"Run a Script", cmdHandler_Script, 9},
    {{"Dialog", "dlg"},"Open a dialog box with any string arg", cmdHandler_Dialog, 9},
    {{"InfoMessage", "imsg"},"Send an info message into chat. Expects <int> <string>, e.g. /imsg 1 test", cmdHandler_InfoMessage, 9},
    {{"SmileX"},"Runs the smlx script SmileX", cmdHandler_SmileX, 9},
    {{"fly", "flying"},"Toggle flying On/Off", cmdHandler_Fly, 9},
    {{"falling"},"Toggle falling On/Off", cmdHandler_Falling, 9},
    {{"sliding"},"Toggle sliding On/Off", cmdHandler_Sliding, 9},
    {{"jumping"},"Toggle jumping On/Off", cmdHandler_Jumping, 9},
    {{"stunned"},"Toggle stunned character On/Off", cmdHandler_Stunned, 9},
    {{"jumppack"},"Toggle jump pack On/Off", cmdHandler_Jumppack, 9},
    {{"setSpeed", "speed"},"Set your player Speed", cmdHandler_SetSpeed, 9},
    {{"setBackupSpd", "BackupSpd"},"Set the Backup Speed", cmdHandler_SetBackupSpd, 9},
    {{"setJumpHeight", "JumpHeight"},"Set the Jump Height", cmdHandler_SetJumpHeight, 9},
    {{"setHP"},"Set the HP value of your character", cmdHandler_SetHP, 9},
    {{"setEnd"},"Set your Endurance", cmdHandler_SetEnd, 9},
    {{"setXP"},"Set your XP", cmdHandler_SetXP, 9},
    {{"setDebt"},"Set your Debt", cmdHandler_SetDebt, 9},
    {{"setInf"},"Set your Influence", cmdHandler_SetInf, 9},
    {{"setLevel"},"Set your Level", cmdHandler_SetLevel, 9},
    {{"setCombatLevel"},"Set your Combat Level", cmdHandler_SetCombatLevel, 9},
    {{"UpdateChar", "CharUpdate", "save"},"Update character information in the database", cmdHandler_UpdateChar, 9},
    {{"DebugChar", "chardebug"},"View your characters debug information", cmdHandler_DebugChar, 9},
    {{"ControlsDisabled"},"Disable controls", cmdHandler_ControlsDisabled, 9},
    {{"updateid"},"Update ID", cmdHandler_UpdateId, 9},
    {{"fullupdate"},"Full Update", cmdHandler_FullUpdate, 9},
    {{"hascontrolid"},"Force the server to acknowledge input ids", cmdHandler_HasControlId, 9},
    {{"setTeam", "setTeamID"},"Set the team idx", cmdHandler_SetTeam, 9},
    {{"setSuperGroup","setSG"},"Set your Super Group", cmdHandler_SetSuperGroup, 9},
    {{"settingsDump","settingsDebug"},"Output settings.cfg to console", cmdHandler_SettingsDump, 9},
    {{"teamDump", "teamDebug"}, "Output team settings to console", cmdHandler_TeamDebug, 9},
    {{"guiDump", "guiDebug"}, "Output gui settings to console", cmdHandler_GUIDebug, 9},
    {{"setWindowVisibility", "setWinVis"}, "", cmdHandler_SetWindowVisibility, 9},
    {{"keybindDump", "keybindDebug"}, "Output keybind settings to console", cmdHandler_KeybindDebug, 9},
    {{"toggleLogging", "log"}, "Modify log categories (e.g. input, teams, ...)", cmdHandler_ToggleLogging, 9},
    {{"friendsDump", "friendsDebug"}, "Output friendlist info to console", cmdHandler_FriendsListDebug, 9},
    {{"damage", "heal"}, "Make current target (or self) take damage/health", cmdHandler_SendFloatingNumbers, 9},
    {{"extrainfo"},"Toggle extra_info", &cmdHandler_ToggleExtraInfo, 9},
    {{"moveinstantly"},"Toggle move_instantly", &cmdHandler_ToggleMoveInstantly, 9},
    {{"setu1"},"Set bitvalue u1. Used for live-debugging.", cmdHandler_SetU1, 9},

    /* Access Level 2 Commands */
    {{"addNpc"},"add <npc_name> with costume [variation] in front of gm", addNpc, 2},
    {{"moveTo"},"set the gm's position to <x> <y> <z>", moveTo, 2},

    /* Access Level 1 Commands */
    {{"cmdlist","commandlist"},"List all accessible commands", cmdHandler_CmdList, 1},
    {{"afk"},"Mark yourself as Away From Keyboard", cmdHandler_AFK, 1},
    {{"whoall"},"Shows who is on the current map ", cmdHandler_WhoAll, 1},
    {{"setTitles"},"Set your title", cmdHandler_SetTitles, 1},
    {{"stuck"},"Free yourself if your character gets stuck", cmdHandler_Stuck, 1},
    {{"lfg"},"Toggle looking for group status ", cmdHandler_LFG, 1},
    {{"motd"},"View the server MOTD", cmdHandler_MOTD, 1},
    {{"i","invite"}, "Invite player to team", cmdHandler_Invite, 1},
    {{"k","kick"}, "Kick player from team", cmdHandler_Kick, 1},
    {{"leaveteam"}, "Leave the team you're currently on", cmdHandler_LeaveTeam, 1},
    {{"findmember"}, "Open Looking For Group Window", cmdHandler_FindMember, 1},
    {{"makeleader","ml"}, "Make another player team leader", cmdHandler_MakeLeader, 1},
    {{"assist"}, "Re-target based on the selected entity's current target", cmdHandler_SetAssistTarget, 1},
    {{"sidekick","sk"}, "Invite player to sidekick", cmdHandler_Sidekick, 1},
    {{"unsidekick","unsk"}, "Remove sidekick/duo relationship", cmdHandler_UnSidekick, 1},
    {{"buffs"}, "Toggle buffs display on team window", cmdHandler_TeamBuffs, 1},
    {{"friend"}, "Add friend to friendlist", cmdHandler_Friend, 1},
    {{"unfriend","estrange"}, "Remove friend from friendlist", cmdHandler_Unfriend, 1},
    {{"friendlist", "fl"}, "Toggle visibility of friendslist", cmdHandler_FriendList, 1},
    /* Access Level 0 Commands :: These are "behind the scenes" and sent by the client */
    {{"team_accept"}, "Accept Team invite", cmdHandler_TeamAccept, 0},
    {{"team_decline"}, "Decline Team invite", cmdHandler_TeamDecline, 0},
    {{"sidekick_accept"}, "Accept Sidekick invite", cmdHandler_SidekickAccept, 0},
    {{"sidekick_decline"}, "Decline Sidekick invite", cmdHandler_SidekickDecline, 0},
    {{"emailheaders"}, "Request Email Headers", cmdHandler_EmailHeaders, 0},
    {{"emailread"}, "Request Email Message with Given ID", cmdHandler_EmailRead, 0},
    {{"emailsend"}, "Send Email", cmdHandler_EmailSend, 0},
    {{"emaildelete"}, "Delete Email with Given ID",cmdHandler_EmailDelete, 0},
};

/************************************************************
 *  Slash Command Handlers
 ***********************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_Script(const QString &cmd, MapClientSession &sess)
{
    QString code = cmd.mid(7, cmd.size() - 7);
    sess.m_current_map->m_scripting_interface->runScript(&sess, code, "user provided script");
}

void cmdHandler_Dialog(const QString &cmd, MapClientSession &sess)
{
    StandardDialogCmd *dlg = new StandardDialogCmd(cmd.mid(4));
    sess.addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
}

void cmdHandler_InfoMessage(const QString &cmd, MapClientSession &sess)
{
    QString msg;
    int cmdType = int(MessageChannel::USER_ERROR);

    int first_space = cmd.indexOf(' ');
    int second_space = cmd.indexOf(' ',first_space+1);
    if(second_space==-1)
        msg = "The /imsg command takes two arguments, a <b>number</b> and a <b>string</b>";
    else
    {
        bool ok = true;
        cmdType = cmd.midRef(first_space+1,second_space-(first_space+1)).toInt(&ok);
        if(!ok || cmdType<1 || cmdType>21)
        {
            msg = "The first /imsg argument must be a <b>number</b> between 1 and 21";
            cmdType = int(MessageChannel::USER_ERROR);
        }
        else
            msg = cmd.mid(second_space+1);
    }
    sendInfoMessage(static_cast<MessageChannel>(cmdType), msg, &sess);
}

void cmdHandler_SmileX(const QString &cmd, MapClientSession &sess) {


    int space = cmd.indexOf(' ');
    QString fileName("scripts/" + cmd.mid(space+1));
    if(!fileName.endsWith(".smlx"))
            fileName.append(".smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        StandardDialogCmd *dlg = new StandardDialogCmd(contents);
        sess.addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
    else {
        QString errormsg = "Failed to load smilex file. \'" + file.fileName() + "\' not found.";
        qCDebug(logSlashCommand) << errormsg;
        sendInfoMessage(MessageChannel::ADMIN, errormsg, &sess);
    }
}

void cmdHandler_Fly(const QString &cmd, MapClientSession &sess) {


    toggleFlying(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_Falling(const QString &cmd, MapClientSession &sess) {


    toggleFalling(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_Sliding(const QString &cmd, MapClientSession &sess) {


    toggleSliding(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_Jumping(const QString &cmd, MapClientSession &sess)
{
    toggleJumping(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_Stunned(const QString &cmd, MapClientSession &sess) {


    toggleStunned(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_Jumppack(const QString &cmd, MapClientSession &sess) {


    toggleJumppack(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetSpeed(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> args(cmd.splitRef(' '));
    float v1 = args.value(1).toFloat();
    float v2 = args.value(2).toFloat();
    float v3 = args.value(3).toFloat();
    setSpeed(*sess.m_ent, v1, v2, v3);

    QString msg = QString("Set Speed to: <%1,%2,%3>").arg(v1).arg(v2).arg(v3);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetBackupSpd(const QString &cmd, MapClientSession &sess)
{
    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setBackupSpd(*sess.m_ent, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetJumpHeight(const QString &cmd, MapClientSession &sess)
{
    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setJumpHeight(*sess.m_ent, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetHP(const QString &cmd, MapClientSession &sess)
{
    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    float maxattrib =sess.m_ent->m_char->m_max_attribs.m_HitPoints;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setHP(*sess.m_ent->m_char,attrib);

    QString msg = QString("Setting HP to: %1 / %2").arg(attrib).arg(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetEnd(const QString &cmd, MapClientSession &sess)
{
    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    float maxattrib = sess.m_ent->m_char->m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(*sess.m_ent->m_char,attrib);

    QString msg = QString("Setting Endurance to: %1 / %2").arg(attrib).arg(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetXP(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();
    uint32_t lvl = getLevel(*sess.m_ent->m_char);

    setXP(*sess.m_ent->m_char, attrib);
    QString msg = "Setting XP to " + QString::number(attrib);

    uint32_t newlvl = getLevel(*sess.m_ent->m_char);
    if(lvl != newlvl)
        msg += " and LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetDebt(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setDebt(*sess.m_ent->m_char, attrib);
    QString msg = QString("Setting XP Debt to %1").arg(attrib);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetInf(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setInf(*sess.m_ent->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetLevel(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setLevel(*sess.m_ent->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Level to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetCombatLevel(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setCombatLevel(*sess.m_ent->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Combat Level to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_UpdateChar(const QString &cmd, MapClientSession &sess)
{
    charUpdateDB(sess.m_ent);

    QString msg = "Updating Character in Database: " + sess.m_ent->name();
    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_DebugChar(const QString &/*cmd*/, MapClientSession &sess)
{
    const Character &chardata(*sess.m_ent->m_char);
    QString msg = "DebugChar: " + sess.m_ent->name()
            + "\n  " + chardata.m_char_data.m_origin_name
            + "\n  " + chardata.m_char_data.m_class_name
            + "\n  map: " + chardata.m_char_data.m_mapName
            + "\n  db_id: " + QString::number(sess.m_ent->m_db_id) + ":" + QString::number(chardata.m_db_id)
            + "\n  idx: " + QString::number(sess.m_ent->m_idx)
            + "\n  access: " + QString::number(sess.m_ent->m_entity_data.m_access_level)
            + "\n  acct: " + QString::number(chardata.m_account_id)
            + "\n  lvl/clvl: " + QString::number(chardata.m_char_data.m_level) + "/" + QString::number(chardata.m_char_data.m_combat_level)
            + "\n  inf: " + QString::number(chardata.m_char_data.m_influence)
            + "\n  xp/debt: " + QString::number(chardata.m_char_data.m_experience_points) + "/" + QString::number(chardata.m_char_data.m_experience_debt)
            + "\n  lfg: " + QString::number(chardata.m_char_data.m_lfg)
            + "\n  afk: " + QString::number(chardata.m_char_data.m_afk)
            + "\n  tgt_idx: " + QString::number(getTargetIdx(*sess.m_ent));
    sess.m_ent->dump();
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_ControlsDisabled(const QString &cmd, MapClientSession &sess)
{
    toggleControlsDisabled(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}


void cmdHandler_UpdateId(const QString &cmd, MapClientSession &sess)
{
    uint8_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUShort();

    setUpdateID(*sess.m_ent, attrib);

    QString msg = "Setting updateID to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_FullUpdate(const QString &cmd, MapClientSession &sess)
{
    toggleFullUpdate(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_HasControlId(const QString &cmd, MapClientSession &sess)
{
    toggleControlId(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetTeam(const QString &cmd, MapClientSession &sess)
{
    uint8_t val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setTeamID(*sess.m_ent, val);

    QString msg = "Set Team ID to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SetSuperGroup(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    int sg_id       = args.value(1).toInt();
    QString sg_name = args.value(2);
    int sg_rank     = args.value(3).toInt();

    setSuperGroup(*sess.m_ent, sg_id, sg_name, sg_rank);

    QString msg = QString("Set SuperGroup:  id: %1  name: %2  rank: %3").arg(QString::number(sg_id), sg_name, QString::number(sg_rank));
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_SettingsDump(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending settings config dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    settingsDump(); // Send settings dump
}

void cmdHandler_TeamDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending team debug to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    sess.m_ent->m_team->dump(); // Send team debug info
}


void cmdHandler_GUIDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending GUISettings dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    sess.m_ent->m_player->m_gui.guiDump(); // Send GUISettings dump
}

void cmdHandler_SetWindowVisibility(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    uint32_t idx = args.value(1).toUInt();
    WindowVisibility val = (WindowVisibility)args.value(2).toInt();

    QString msg = "Toggling " + QString::number(idx) +  " GUIWindow visibility: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    sess.m_ent->m_player->m_gui.m_wnds.at(idx).setWindowVisibility(val); // Set WindowVisibility
    sess.m_ent->m_player->m_gui.m_wnds.at(idx).guiWindowDump(); // for debugging
}

void cmdHandler_KeybindDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending Keybinds dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    sess.m_ent->m_player->m_keybinds.keybindsDump(); // Send GUISettings dump
}

void cmdHandler_ToggleLogging(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    args.removeFirst();

    QString msg = "Toggle logging of categories: " + args.join(" ");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    for (auto category : args)
        toggleLogging(category); // Toggle each category listed
}

void cmdHandler_FriendsListDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending FriendsList dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);

    dumpFriends(*sess.m_ent); // Send FriendsList dump
}

void cmdHandler_SendFloatingNumbers(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    QString msg; // result messages
    int first_space  = cmd.indexOf(' ');
    int second_space = cmd.indexOf(' ',first_space+1);
    int third_space  = cmd.indexOf(' ',second_space+1);

    bool ok1 = true;
    bool ok2 = true;
    uint32_t runtimes   = cmd.midRef(first_space+1, second_space-(first_space+1)).toInt(&ok1);
    float amount        = cmd.midRef(second_space+1, third_space-(second_space+1)).toFloat(&ok2);
    QString name        = cmd.midRef(third_space+1).toString();

    // reign in the insanity
    if(runtimes<=0)
        runtimes = 1;
    else if(runtimes > 5)
        runtimes = 5;

    if(!ok1 || !ok2 || name.isEmpty())
    {
        msg = "FloatingNumbers takes three arguments: `/damage <number_times_to_run> <damage_amount> <target_name>`";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, &sess);
        return;
    }

    tgt = getEntity(&sess,name); // get Entity by name

    if(tgt == nullptr)
    {
        msg = "FloatingNumbers target " + name + " cannot be found.";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, &sess);
        return;
    }

    for(uint32_t i = 0; i<runtimes; i++)
    {
        sendFloatingNumbers(sess.m_ent, tgt->m_idx, int(amount));

        setHP(*tgt->m_char, getHP(*tgt->m_char)-amount); // deal dmg

        if(amount >= 0) // damage
        {
            msg = QString("%1 deals %2 points of damage to %3.").arg(sess.m_ent->name()).arg(amount).arg(name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You deal %1 points of damage to %2.").arg(QString::number(amount), name);
            sendInfoMessage(MessageChannel::DAMAGE, msg, &sess);
            msg = QString("%1 has dealt you %2 points of damage!").arg(sess.m_ent->name()).arg(amount);
            sendInfoMessage(MessageChannel::DAMAGE, msg, tgt->m_client);
        }
        else
        {
            msg = QString("%1 heals %2 points of damage from %3.").arg(sess.m_ent->name()).arg(-amount).arg(name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You heal %1 points of damage from %2.").arg(-amount).arg(name);
            sendInfoMessage(MessageChannel::TEAM, msg, &sess); // TEAM for green
            msg = QString("%1 has healed %2 points of damage from you!").arg(sess.m_ent->name()).arg(-amount);
            sendInfoMessage(MessageChannel::TEAM, msg, tgt->m_client); // TEAM for green
        }
    }
}

void cmdHandler_ToggleExtraInfo(const QString &cmd, MapClientSession &sess)
{
    toggleExtraInfo(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_ToggleMoveInstantly(const QString &cmd, MapClientSession &sess)
{
    toggleMoveInstantly(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

// Slash commands for setting bit values
void cmdHandler_SetU1(const QString &cmd, MapClientSession &sess)
{
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu1(*sess.m_ent, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 2 Commands
void addNpc(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    int variation = 0;
    if(cmd.contains('"')) // assume /addNpc "A guy in a hat" 1
    {
        int start_idx = cmd.indexOf('"');
        int end_idx = cmd.indexOf('"',start_idx+1);
        parts.push_back(cmd.midRef(0,start_idx-1));
        if(end_idx!=-1)
            parts.push_back(cmd.midRef(start_idx+1,end_idx-start_idx-2));
        if(cmd.midRef(end_idx+1).size()>0)
            parts.push_back(cmd.midRef(end_idx+1));
    }
    else
        parts = cmd.splitRef(' ');
    if(parts.size()>2) // assume /addNpc Monsterifier 1
        variation = parts[2].toInt();
    if(parts.size()<2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:"<<cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:"+cmd, &sess);
        return;
    }
    glm::vec3 gm_loc = sess.m_ent->m_entity_data.m_pos;
    const NPCStorage & npc_store(sess.m_current_map->serverData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(parts[1]);
    if(!npc_def)
    {
        sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for:"+parts[1], &sess);
        return;
    }
    glm::vec3 offset = glm::vec3 {2,0,1};
    int idx = npc_store.npc_idx(npc_def);
    Entity *e = sess.m_current_map->m_entities.CreateNpc(*npc_def,idx,variation);
    forcePosition(*e,gm_loc + offset);
    e->m_velocity = {0,0,0};
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1").arg(e->m_idx), &sess);
}

void moveTo(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');
    if(parts.size()<4)
    {
        qCDebug(logSlashCommand) << "Bad invocation:"<<cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:"+cmd, &sess);
    }
    glm::vec3 new_pos {
      parts[1].toFloat(),
      parts[2].toFloat(),
      parts[3].toFloat()
    };
    forcePosition(*sess.m_ent,new_pos);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("New position set"), &sess);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_CmdList(const QString &cmd, MapClientSession &sess)
{

    QString msg = "Below is a list of all slash commands that your account can access. They are not case sensitive.\n";
    QString msg_dlg = "<face heading><span align=center><color #ff0000>Command List</color></span></face><br>\n<br>\n";

    for (const auto &sc : g_defined_slash_commands)
    {
        uint32_t alvl = getAccessLevel(*sess.m_ent);
        if (alvl < sc.m_required_access_level)
            continue;

        // We don't want to show acess_level 0 (Under the Hood commands)
        if (sc.m_required_access_level == 0)
            continue;

        // Use msg for std out, msg_dlg for ingame dialog box
        msg += "\t" + sc.m_valid_prefixes.join(", ") + " [" + QString::number(sc.m_required_access_level) +
               "]:\t" + sc.m_help_text + "\n";
        msg_dlg += QString("<color #ffCC99><i>%1</i></color>[<color #66ffff>%2</color>]: %3<br>")
                       .arg(sc.m_valid_prefixes.join(", "))
                       .arg(sc.m_required_access_level)
                       .arg(sc.m_help_text);
    }

    // Dialog output
    StandardDialogCmd *dlg = new StandardDialogCmd(msg_dlg);
    sess.addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    // CMD line (debug) output
    qCDebug(logSlashCommand).noquote() << cmd << ":\n" << msg;
}

void cmdHandler_AFK(const QString &cmd, MapClientSession &sess)
{
    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    toggleAFK(*sess.m_ent->m_char, val);

    QString msg = "Setting afk message to: " + val;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::EMOTE, msg, &sess);
}

void cmdHandler_WhoAll(const QString &/*cmd*/, MapClientSession &sess)
{
    MapInstance *     mi  = sess.m_current_map;

    QString msg = "Players on this map:\n";

    for (MapClientSession *cl : mi->m_session_store)
    {
        Character &c(*cl->m_ent->m_char);
        QString    name      = cl->m_ent->name();
        QString    lvl       = QString::number(getLevel(c));
        QString    clvl      = QString::number(getCombatLevel(c));
        QString    origin    = getOrigin(c);
        QString    archetype = QString(getClass(c)).remove("Class_");

        // Format: character_name "lvl" level "clvl" combat_level origin archetype
        msg += QString("%1 lvl %2 clvl %3 %4 %5\n").arg(name,lvl,clvl,origin,archetype);
    }

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, &sess);
}

void cmdHandler_SetTitles(const QString &cmd, MapClientSession &sess)
{
    bool        prefix;
    QString     msg, generic, origin, special;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    if (cmd.toLower() == "settitles")
    {
        setTitles(*sess.m_ent->m_char);
        msg = "Titles reset to nothing";
    }
    else
    {
        prefix  = !args.value(1).isEmpty();
        generic = args.value(2);
        origin  = args.value(3);
        special = args.value(4);
        setTitles(*sess.m_ent->m_char, prefix, generic, origin, special);
        msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;
    }
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, &sess);
}

void cmdHandler_Stuck(const QString &cmd, MapClientSession &sess)
{
    // TODO: Implement true move-to-safe-location-nearby logic
    forcePosition(*sess.m_ent,sess.m_current_map->closest_safe_location(sess.m_ent->m_entity_data.m_pos));

    QString msg = QString("Resetting location to default spawn (%1,%2,%3)")
                      .arg(sess.m_ent->m_entity_data.m_pos.x)
                      .arg(sess.m_ent->m_entity_data.m_pos.y)
                      .arg(sess.m_ent->m_entity_data.m_pos.z);
    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, &sess);
}

void cmdHandler_LFG(const QString &cmd, MapClientSession &sess)
{
    toggleLFG(*sess.m_ent);
    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, &sess);
}

void cmdHandler_MOTD(const QString &/*cmd*/, MapClientSession &sess)
{
    sendServerMOTD(&sess);
    QString msg = "Opening Server MOTD";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, &sess);
}

void cmdHandler_Invite(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;
    QString msg;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(&sess,getTargetIdx(*sess.m_ent));
        name = tgt->name();
    }
    else
        tgt = getEntity(&sess,name);

    if(tgt == nullptr)
        return;

    if(tgt->m_has_team)
    {
        msg = tgt->name() + " is already on a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, &sess);
        return;
    }

    if(tgt->name() == sess.m_name)
    {
        msg = "You cannot invite yourself to a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, &sess);
        return;
    }

    if(sess.m_ent->m_has_team && sess.m_ent->m_team != nullptr)
    {
        if(!sess.m_ent->m_team->isTeamLeader(sess.m_ent))
        {
            msg = "Only the team leader can invite players to the team.";
            qCDebug(logTeams) << sess.m_ent->name() << msg;
            sendInfoMessage(MessageChannel::TEAM, msg, &sess);
            return;
        }
    }

    sendTeamOffer(sess.m_ent,tgt);
}

void cmdHandler_Kick(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;
    QString msg;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(&sess,getTargetIdx(*sess.m_ent));
        name = tgt->name();
    }
    else
        tgt = getEntity(&sess,name);

    if(tgt == nullptr)
        return;

    if(kickTeam(*tgt))
        msg = "Kicking " + name + " from team.";
    else
        msg = "Failed to kick " + name;

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, &sess);
}

void cmdHandler_LeaveTeam(const QString &/*cmd*/, MapClientSession &sess)
{
    leaveTeam(*sess.m_ent);
    QString msg = "Leaving Team";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, &sess);
}

void cmdHandler_FindMember(const QString &/*cmd*/, MapClientSession &sess)
{
    findTeamMember(*sess.m_ent);
    QString msg = "Finding Team Member";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::CHAT_TEXT, msg, &sess);
}


void cmdHandler_MakeLeader(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;
    QString msg;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(&sess,getTargetIdx(*sess.m_ent));
        name = tgt->name();
    }
    else
        tgt = getEntity(&sess,name);

    if(tgt == nullptr)
        return;

    if(makeTeamLeader(*sess.m_ent,*tgt))
        msg = "Making " + name + " team leader.";
    else
        msg = "Failed to make " + name + " team leader.";

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, &sess);
}

void cmdHandler_SetAssistTarget(const QString &/*cmd*/, MapClientSession &sess)
{
    Entity *target_ent = getEntity(&sess, getTargetIdx(*sess.m_ent));
    if(target_ent == nullptr)
        return;

    uint32_t new_target = getTargetIdx(*target_ent);
    if(new_target == 0)
        return;

    setTarget(*sess.m_ent, new_target);

    QString msg = "Now targeting " + target_ent->name() + "'s target";
    sendInfoMessage(MessageChannel::TEAM, msg, &sess);
    qCDebug(logSlashCommand).noquote() << msg;
}

void cmdHandler_Sidekick(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(&sess,getTargetIdx(*sess.m_ent));
        name = tgt->name();
    }
    else
        tgt = getEntity(&sess,name);

    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    inviteSidekick(*sess.m_ent,*tgt);
}

void cmdHandler_UnSidekick(const QString &/*cmd*/, MapClientSession &sess)
{
    if(sess.m_ent->m_char->isEmpty())
        return;

    removeSidekick(*sess.m_ent);
}

void cmdHandler_TeamBuffs(const QString & /*cmd*/, MapClientSession &sess)
{
    toggleTeamBuffs(*sess.m_ent->m_player);

    QString msg = "Toggling Team Buffs display mode.";
    qCDebug(logSlashCommand).noquote() << msg;
}

void cmdHandler_Friend(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(&sess,getTargetIdx(*sess.m_ent));
        name = tgt->name();
    }
    else
        tgt = getEntity(&sess,name);

    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    addFriend(*sess.m_ent,*tgt);
}

void cmdHandler_Unfriend(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(&sess,getTargetIdx(*sess.m_ent));
        name = tgt->name();
    }
    else
        tgt = getEntity(&sess,name);

    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    // TODO: Implement getCharacterFromDB(name) if target is not online.

    removeFriend(*sess.m_ent,*tgt);
}

void cmdHandler_FriendList(const QString &/*cmd*/, MapClientSession &sess)
{
    if(sess.m_ent->m_char->isEmpty())
        return;

    toggleFriendList(*sess.m_ent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 0 Commands
void cmdHandler_TeamAccept(const QString &cmd, MapClientSession &sess)
{
    // game command: "team_accept \"From\" to_db_id to_db_id \"To\""

    QString msgfrom = "Something went wrong with TeamAccept.";
    QString msgtgt = "Something went wrong with TeamAccept.";
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    QString from_name       = args.value(1);
    uint32_t tgt_db_id      = args.value(2).toUInt();
    uint32_t tgt_db_id_2    = args.value(3).toUInt(); // always the same?
    QString tgt_name        = args.value(4);

    if(tgt_db_id != tgt_db_id_2)
        qWarning() << "TeamAccept db_ids do not match!";

    Entity *from_ent = getEntity(&sess,from_name);
    if(from_ent == nullptr)
        return;

    if(inviteTeam(*from_ent,*sess.m_ent))
    {
        msgfrom = "Inviting " + tgt_name + " to team.";
        msgtgt = "Joining " + from_name + "'s team.";

    }
    else
    {
        msgfrom = "Failed to invite " + tgt_name + ". They are already on a team.";
    }

    qCDebug(logSlashCommand).noquote() << msgfrom;
    sendInfoMessage(MessageChannel::TEAM, msgfrom, from_ent->m_client);
    sendInfoMessage(MessageChannel::TEAM, msgtgt, &sess);
}

void cmdHandler_TeamDecline(const QString &cmd, MapClientSession &sess)
{
    // game command: "team_decline \"From\" to_db_id \"To\""
    QString msg;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    QString from_name   = args.value(1);
    uint32_t tgt_db_id  = args.value(2).toUInt();
    QString tgt_name    = args.value(3);

    Entity *from_ent = getEntity(&sess,from_name);
    if(from_ent == nullptr)
        return;

    msg = tgt_name + " declined a team invite from " + from_name + QString::number(tgt_db_id);
    qCDebug(logSlashCommand).noquote() << msg;

    msg = tgt_name + " declined your team invite."; // to sender
    sendInfoMessage(MessageChannel::TEAM, msg, from_ent->m_client);
    msg = "You declined the team invite from " + from_name; // to target
    sendInfoMessage(MessageChannel::TEAM, msg, &sess);
}

void cmdHandler_SidekickAccept(const QString &/*cmd*/, MapClientSession &sess)
{
    uint32_t db_id  = sess.m_ent->m_char->m_char_data.m_sidekick.m_db_id;
    Entity *tgt     = getEntityByDBID(&sess,db_id);

    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    addSidekick(*sess.m_ent,*tgt);
}

void cmdHandler_SidekickDecline(const QString &/*cmd*/, MapClientSession &sess)
{
    sess.m_ent->m_char->m_char_data.m_sidekick.m_db_id = 0;
}

void cmdHandler_EmailHeaders(const QString & /*cmd*/, MapClientSession &sess)
{
    sendEmailHeaders(sess.m_ent);
    QString msg = "Sent Email Headers";
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_EmailRead(const QString &cmd, MapClientSession &sess)
{
    int id = cmd.midRef(cmd.indexOf(' ')+1).toInt();

    readEmailMessage(sess.m_ent, id);

    QString msg = "Opening Email ID: " + QString::number(id);
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

void cmdHandler_EmailSend(const QString &cmd, MapClientSession &sess){

    QVector<QStringRef> args(cmd.splitRef(' '));

    //storeEmailInDB(src, args);

    QString msg = "Email Sent";
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, &sess);
}

void cmdHandler_EmailDelete(const QString &cmd, MapClientSession &sess)
{
    int id = cmd.midRef(cmd.indexOf(' ')+1).toInt();

    //deleteEmailFromDB(id);

    QString msg = "Email Deleted ID: " + QString::number(id);
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, &sess);
}

bool canAccessCommand(const SlashCommand &cmd, MapClientSession &src)
{
    uint32_t alvl = getAccessLevel(*src.m_ent);
    if(alvl >= cmd.m_required_access_level)
        return true;

    QString msg = "You do not have adequate permissions to use the command: " + cmd.m_valid_prefixes.first();
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, &src);
    return false;
}
} // end of anonymous namespace

/*
 * runCommand for executing commands on MapClientSession
 */
void runCommand(const QString &str, MapClientSession &e)
{
    for (const auto &cmd : g_defined_slash_commands)
    {
        if (cmd.m_valid_prefixes.contains(str.split(' ').front(), Qt::CaseInsensitive))
        {
            if (!canAccessCommand(cmd, e))
                return; // no access, so return early
            cmd.m_handler(str, e);
            return; // return here to avoid unknown command msg
        }
    }
    qCDebug(logSlashCommand) << "Unknown game command:" << str;
}

//! @}

