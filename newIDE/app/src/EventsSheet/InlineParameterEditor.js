// @flow
import * as React from 'react';
import InlinePopover from './InlinePopover';
import ParameterRenderingService from './InstructionEditor/ParameterRenderingService';
import {
  type ResourceSource,
  type ChooseResourceFunction,
} from '../ResourcesList/ResourceSource.flow';
const gd = global.gd;

type Props = {|
  project: gdProject,
  layout: gdLayout,

  open: boolean,
  onRequestClose: () => void,
  onChange: (string) => void,

  instruction: gdInstruction,
  isCondition: boolean,
  parameterIndex: number,

  anchorEl: ?any,

  resourceSources: Array<ResourceSource>,
  onChooseResource: ChooseResourceFunction,
|};

type State = {|
  isValid: boolean,
  parameterMetadata: ?gdParameterMetadata,
  instruction: ?gdInstruction,
  ParameterComponent: ?any,
|};

export default class InlineParameterEditor extends React.Component<Props, State> {
  state = {
    isValid: false,
    parameterMetadata: null,
    ParameterComponent: null,
    instruction: null,
  };

  _field: ?any;

  componentWillReceiveProps(newProps: Props) {
    if (
      (newProps.open && !this.props.open) ||
      newProps.instruction !== this.props.instruction
    ) {
      this._loadComponentFromInstruction(newProps);
    }
  }

  _unload() {
    this.setState({
      ParameterComponent: null,
      parameterMetadata: null,
      instruction: null,
    });
  }

  _loadComponentFromInstruction(props: Props) {
    const { project, isCondition, instruction, parameterIndex } = props;
    if (!instruction) return this._unload();

    const type = instruction.getType();
    const instructionMetadata = isCondition
      ? gd.MetadataProvider.getConditionMetadata(
          project.getCurrentPlatform(),
          type
        )
      : gd.MetadataProvider.getActionMetadata(
          project.getCurrentPlatform(),
          type
        );

    if (parameterIndex >= instructionMetadata.getParametersCount())
      return this._unload();

    const parameterMetadata = instructionMetadata.getParameter(parameterIndex);
    const ParameterComponent = ParameterRenderingService.getParameterComponent(
      parameterMetadata.getType()
    );
    this.setState(
      {
        ParameterComponent,
        parameterMetadata,
      },
      () => {
        if (this._field && this._field.focus) this._field.focus();
      }
    );
  }

  render() {
    if (!this.state.ParameterComponent || !this.props.open) return null;

    const { ParameterComponent } = this.state;

    return (
      <InlinePopover
        open={this.props.open}
        anchorEl={this.props.anchorEl}
        onRequestClose={this.props.onRequestClose}
      >
        <ParameterComponent
          parameterMetadata={this.state.parameterMetadata}
          project={this.props.project}
          layout={this.props.layout}
          value={this.props.instruction.getParameter(this.props.parameterIndex)}
          instructionOrExpression={this.props.instruction}
          key={this.props.instruction.ptr}
          onChange={this.props.onChange}
          ref={field => (this._field = field)}
          parameterRenderingService={ParameterRenderingService}
          isInline
          resourceSources={this.props.resourceSources}
          onChooseResource={this.props.onChooseResource}
        />
      </InlinePopover>
    );
  }
}
