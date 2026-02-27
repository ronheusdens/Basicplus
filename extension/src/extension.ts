import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext) {
    console.log('Basic++ extension activated');

    // Register command to run program
    let runCommand = vscode.commands.registerCommand('basicpp.runProgram', () => {
        vscode.window.showInformationMessage('Run Basic++ Program');
    });

    // Register command to debug program
    let debugCommand = vscode.commands.registerCommand('basicpp.debugProgram', () => {
        vscode.window.showInformationMessage('Debug Basic++ Program');
    });

    context.subscriptions.push(runCommand);
    context.subscriptions.push(debugCommand);
}

export function deactivate() {
    console.log('Basic++ extension deactivated');
}
