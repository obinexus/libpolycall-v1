package org.obinexus.cli;

import org.obinexus.cli.commands.InfoCommand;
import org.obinexus.cli.commands.TestCommand;
import org.obinexus.cli.commands.TelemetryCommand;
import picocli.CommandLine;

/**
 * Main CLI entry point for Java PolyCall
 */
@CommandLine.Command(
    name = "java-polycall",
    description = "LibPolyCall Trial v1 Java Binding CLI",
    subcommands = {InfoCommand.class, TestCommand.class, TelemetryCommand.class}
)
public class Main {
    
    public static void main(String[] args) {
        int exitCode = new CommandLine(new Main()).execute(args);
        System.exit(exitCode);
    }
}
