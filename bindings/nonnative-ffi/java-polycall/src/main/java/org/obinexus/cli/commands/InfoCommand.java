package org.obinexus.cli.commands;

import picocli.CommandLine;
import java.util.concurrent.Callable;

@CommandLine.Command(name = "info", description = "Display protocol information")
public class InfoCommand implements Callable<Integer> {
    
    @Override
    public Integer call() throws Exception {
        System.out.println("Java PolyCall - LibPolyCall Trial v1 Java Binding");
        System.out.println("Protocol-compliant adapter for polycall.exe runtime");
        System.out.println("Version: 1.0.0");
        return 0;
    }
}
