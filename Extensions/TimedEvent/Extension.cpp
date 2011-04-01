/**

Game Develop - Timed Event Extension
Copyright (c) 2011 Florian Rival (Florian.Rival@gmail.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.

*/

#include "GDL/ExtensionBase.h"
#include "GDL/Version.h"
#include <boost/version.hpp>
#include "TimedEvent.h"
#include "TimedEventActions.h"
#include "GDL/RuntimeScene.h"

/**
 * This class declare information about the extension.
 */
class Extension : public ExtensionBase
{
    public:

        /**
         * Constructor of an extension declares everything the extension contains : Objects, actions, conditions and expressions.
         */
        Extension()
        {
            DECLARE_THE_EXTENSION("TimedEvent",
                                  _("Evenements � retardement"),
                                  _("Extension permettant d'utiliser des �v�nements agissant comme des fonctions."),
                                  "Compil Games",
                                  "zlib/libpng License ( Open Source )")

            DECLARE_EVENT("TimedEvent",
                          _("Evenement � retardement"),
                          _("Evenement qui ne se d�clenche qu'au bout de l'accumulation d'un certain temps"),
                          "",
                          "Extensions/timedevent16.png",
                          TimedEvent)

            DECLARE_END_EVENT()

            DECLARE_ACTION("ResetTimedEvent",
                           _("Remettre � z�ro un �v�nement retard�"),
                           _("Remet � z�ro un �v�nement � retardement"),
                           _("Remettre � z�ro le(s) �v�nement(s) retard�(s) _PARAM0_"),
                           _("Evenements retard�s"),
                           "Extensions/timedevent24.png",
                           "Extensions/timedevent16.png",
                           &ActResetTimedEvent);

                DECLARE_PARAMETER("", _("Nom"), false, "")

            DECLARE_END_ACTION()

            DECLARE_ACTION("ResetTimedEventAndSubs",
                           _("Remettre � z�ro ainsi que les sous �v�nements"),
                           _("Remet � z�ro un �v�nement retard�, ainsi que tous les sous �v�nements � retardement qu'il contient."),
                           _("Remettre � z�ro le(s) �v�nement(s) retard�(s) _PARAM0_ et les sous �v�nements"),
                           _("Evenements retard�s"),
                           "Extensions/timedevent24.png",
                           "Extensions/timedevent16.png",
                           &ActResetTimedEventAndSubs);

                DECLARE_PARAMETER("", _("Nom"), false, "")

            DECLARE_END_ACTION()

            CompleteCompilationInformation();
        };
        virtual ~Extension() {};

        #if defined(GD_IDE_ONLY)
        bool HasDebuggingProperties() const { return true; };

        void GetPropertyForDebugger(RuntimeScene & scene, unsigned int propertyNb, std::string & name, std::string & value) const
        {
            unsigned int i;
            std::map < std::string, TimedEvent* >::const_iterator end = TimedEvent::timedEventsList[&scene].end();
            for (std::map < std::string, TimedEvent* >::iterator iter = TimedEvent::timedEventsList[&scene].begin();iter != end;++iter)
            {
                if ( propertyNb == i )
                {
                    if ( !iter->second ) return;
                    name = iter->second->GetName().empty() ? string(_("Sans nom").mb_str()) : iter->second->GetName();
                    value = ToString(iter->second->GetTime())+"s";

                    return;
                }

                ++i;
            }
        }

        bool ChangeProperty(RuntimeScene & scene, unsigned int propertyNb, std::string newValue)
        {
            unsigned int i;
            std::map < std::string, TimedEvent* >::const_iterator end = TimedEvent::timedEventsList[&scene].end();
            for (std::map < std::string, TimedEvent* >::iterator iter = TimedEvent::timedEventsList[&scene].begin();iter != end;++iter)
            {
                if ( propertyNb == i && iter->second)
                {
                    iter->second->SetTime(ToFloat(newValue));

                    return true;
                }

                ++i;
            }

            return false;
        }

        unsigned int GetNumberOfProperties(RuntimeScene & scene) const
        {
            return TimedEvent::timedEventsList[&scene].size();
        }
        #endif

    protected:
    private:

        /**
         * This function is called by Game Develop so
         * as to complete information about how the extension was compiled ( which libs... )
         * -- Do not need to be modified. --
         */
        void CompleteCompilationInformation()
        {
            #if defined(GD_IDE_ONLY)
            compilationInfo.runtimeOnly = false;
            #else
            compilationInfo.runtimeOnly = true;
            #endif

            #if defined(__GNUC__)
            compilationInfo.gccMajorVersion = __GNUC__;
            compilationInfo.gccMinorVersion = __GNUC_MINOR__;
            compilationInfo.gccPatchLevel = __GNUC_PATCHLEVEL__;
            #endif

            compilationInfo.boostVersion = BOOST_VERSION;

            compilationInfo.sfmlMajorVersion = 2;
            compilationInfo.sfmlMinorVersion = 0;

            #if defined(GD_IDE_ONLY)
            compilationInfo.wxWidgetsMajorVersion = wxMAJOR_VERSION;
            compilationInfo.wxWidgetsMinorVersion = wxMINOR_VERSION;
            compilationInfo.wxWidgetsReleaseNumber = wxRELEASE_NUMBER;
            compilationInfo.wxWidgetsSubReleaseNumber = wxSUBRELEASE_NUMBER;
            #endif

            compilationInfo.gdlVersion = RC_FILEVERSION_STRING;
            compilationInfo.sizeOfpInt = sizeof(int*);

            compilationInfo.informationCompleted = true;
        }
};

/**
 * Used by Game Develop to create the extension class
 * -- Do not need to be modified. --
 */
extern "C" ExtensionBase * GD_EXTENSION_API CreateGDExtension() {
    return new Extension;
}

/**
 * Used by Game Develop to destroy the extension class
 * -- Do not need to be modified. --
 */
extern "C" void GD_EXTENSION_API DestroyGDExtension(ExtensionBase * p) {
    delete p;
}