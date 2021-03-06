/*
 * Copyright (C) Kreogist Dev Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KNSHORTCUTMANAGER_H
#define KNSHORTCUTMANAGER_H

#include <QHash>

#include <QObject>

#define knShortcut (KNShortcutManager::instance())

class QAction;
class KNConfigure;
/*!
 * \brief The KNShortcutManager class is a manager of the application level
 * shortcut keys, aka hotkeys.\n
 * This manager is actually managing a bunch of actions. The manager will
 * allocate the shortcut for those actions.
 */
class KNShortcutManager : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Get the singleton instance of the shortcut manager. Before the
     * initial() has been called, this function will return nullptr.
     * \return The instance of shortcut manager.
     */
    static KNShortcutManager *instance();

    /*!
     * \brief Initial the manager, generate the instance with the given parent
     * object.\n
     * Only the first time will create a instance.
     */
    static void initial(QObject *parent = 0);

    /*!
     * \brief Append an action to the manager. The manager won't take the
     * relationship of the original action. All the shortcut of this action will
     * be processed by the manager.
     * \param identifier The identifier of the action.
     * \param action The action pointer.
     */
    void append(const QString &identifier, QAction *action);

    /*!
     * \brief Set the shortcut manager configure.
     * \param shortcutConfigure The configure object pointer.
     */
    void setShortcutConfigure(KNConfigure *shortcutConfigure);

signals:

public slots:

private slots:
    void onConfigureChanged();

private:
    static KNShortcutManager *m_instance;
    explicit KNShortcutManager(QObject *parent = 0);
    //Disable the copy of the instance.
    KNShortcutManager(const KNShortcutManager &);
    KNShortcutManager(KNShortcutManager &&);

    QHash<uint, QAction *> m_actionMapper;
    KNConfigure *m_shortcutConfigure;
};

#endif // KNSHORTCUTMANAGER_H
