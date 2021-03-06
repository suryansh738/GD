// @flow
import 'element-closest';
import React from 'react';
import ReactDOM from 'react-dom';
import MainFrame from './MainFrame';
import Window from './Utils/Window';
import ExportDialog from './Export/ExportDialog';
import CreateProjectDialog from './ProjectCreation/CreateProjectDialog';
import Authentification from './Utils/GDevelopServices/Authentification';
import {
  sendProgramOpening,
  installAnalyticsEvents,
} from './Utils/Analytics/EventSender';
import { installRaven } from './Utils/Analytics/Raven';
import { installFullstory } from './Utils/Analytics/Fullstory';
import { unregister } from './registerServiceWorker';
import './UI/iconmoon-font.css'; // Styles for Iconmoon font.
import 'react-virtualized/styles.css'; // Styles for react-virtualized Table

// Import for browser only IDE
import BrowserExamples from './ProjectCreation/BrowserExamples';
import BrowserProjectOpener from './ProjectsStorage/BrowserProjectOpener';
import BrowserSaveDialog from './ProjectsStorage/BrowserSaveDialog';
import BrowserIntroDialog from './MainFrame/BrowserIntroDialog';
import browserResourceSources from './ResourcesList/BrowserResourceSources';
import BrowserS3PreviewLauncher from './Export/BrowserExporters/BrowserS3PreviewLauncher';
import { getBrowserExporters } from './Export/BrowserExporters';

// Import for Electron powered IDE.
import ExternalEditor from './ExternalEditor';
import optionalRequire from './Utils/OptionalRequire.js';
import LocalExamples from './ProjectCreation/LocalExamples';
import localResourceSources from './ResourcesList/LocalResourceSources';
import LocalProjectWriter from './ProjectsStorage/LocalProjectWriter';
import LocalProjectOpener from './ProjectsStorage/LocalProjectOpener';
import LocalPreviewLauncher from './Export/LocalExporters/LocalPreviewLauncher';
import { getLocalExporters } from './Export/LocalExporters';
import ElectronEventsBridge from './MainFrame/ElectronEventsBridge';
import LocalIntroDialog from './MainFrame/LocalIntroDialog';
const electron = optionalRequire('electron');

const authentification = new Authentification();
installAnalyticsEvents(authentification);
installRaven();
installFullstory();

Window.setUpContextMenu();

let app = null;

if (electron) {
  const appArguments = Window.getArguments();
  if (appArguments['server-port']) {
    app = (
      <ExternalEditor
        serverPort={appArguments['server-port']}
        isIntegrated={appArguments['mode'] === 'integrated'}
        editor={appArguments['editor']}
        editedElementName={appArguments['edited-element-name']}
      >
        <MainFrame
          resourceSources={localResourceSources}
          authentification={authentification}
          onReadFromPathOrURL={() => Promise.reject("Should never be called")}
        />
      </ExternalEditor>
    );
  } else {
    app = (
      <ElectronEventsBridge>
        <MainFrame
          previewLauncher={<LocalPreviewLauncher />}
          exportDialog={<ExportDialog exporters={getLocalExporters()} />}
          createDialog={
            <CreateProjectDialog examplesComponent={LocalExamples} />
          }
          introDialog={<LocalIntroDialog />}
          onSaveProject={LocalProjectWriter.saveProject}
          onChooseProject={LocalProjectOpener.chooseProjectFile}
          onReadFromPathOrURL={LocalProjectOpener.readProjectJSONFile}
          resourceSources={localResourceSources}
          authentification={authentification}
        />
      </ElectronEventsBridge>
    );
  }
} else {
  app = (
    <MainFrame
      previewLauncher={<BrowserS3PreviewLauncher />}
      exportDialog={<ExportDialog exporters={getBrowserExporters()} />}
      createDialog={<CreateProjectDialog examplesComponent={BrowserExamples} />}
      introDialog={<BrowserIntroDialog />}
      saveDialog={<BrowserSaveDialog />}
      onReadFromPathOrURL={BrowserProjectOpener.readInternalFile}
      resourceSources={browserResourceSources}
      authentification={authentification}
    />
  );
}

const rootElement = document.getElementById('root');
if (rootElement) ReactDOM.render(app, rootElement);
else console.error('No root element defined in index.html');

// registerServiceWorker();
unregister();
sendProgramOpening();
