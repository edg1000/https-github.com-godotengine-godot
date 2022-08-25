using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;

namespace Godot.SourceGenerators
{
    [Generator]
    public class ScriptSerializationGenerator : IIncrementalGenerator
    {
        public void Initialize(IncrementalGeneratorInitializationContext context)
        {
            var areGodotSourceGeneratorsDisabled = context.AnalyzerConfigOptionsProvider.Select(static (provider, _) => provider.AreGodotSourceGeneratorsDisabled());

            var godotClasses = context.SyntaxProvider.CreateValuesProviderForGodotClasses();

            var values = areGodotSourceGeneratorsDisabled
                .Combine(context.CompilationProvider)
                .Combine(godotClasses.Collect());

            context.RegisterSourceOutput(values, static (spc, source) =>
            {
                (bool areGodotSourceGeneratorsDisabled, Compilation compilation) = source.Left;
                var godotClasses = source.Right;

                if (areGodotSourceGeneratorsDisabled)
                    return;

                Execute(spc, compilation, godotClasses);
            });
        }

        private static void Execute(SourceProductionContext context, Compilation compilation, ImmutableArray<GodotClassData> godotClassDatas)
        {
            INamedTypeSymbol[] godotClasses = godotClassDatas.Where(x =>
            {
                // Report and skip non-partial classes
                if (x.DeclarationSyntax.IsPartial())
                {
                    if (x.DeclarationSyntax.IsNested() && !x.DeclarationSyntax.AreAllOuterTypesPartial(out var typeMissingPartial))
                    {
                        Common.ReportNonPartialGodotScriptOuterClass(context, compilation, typeMissingPartial);
                        return false;
                    }

                    return true;
                }

                Common.ReportNonPartialGodotScriptClass(context, x.DeclarationSyntax, x.Symbol);
                return false;
            }).Select(x => x.Symbol)
                .Distinct<INamedTypeSymbol>(SymbolEqualityComparer.Default)
                .ToArray();

            if (godotClasses.Length > 0)
            {
                var typeCache = new MarshalUtils.TypeCache(compilation);

                foreach (var godotClass in godotClasses)
                {
                    VisitGodotScriptClass(context, typeCache, godotClass);
                }
            }
        }

        private static void VisitGodotScriptClass(
            SourceProductionContext context,
            MarshalUtils.TypeCache typeCache,
            INamedTypeSymbol symbol
        )
        {
            INamespaceSymbol namespaceSymbol = symbol.ContainingNamespace;
            string classNs = namespaceSymbol != null && !namespaceSymbol.IsGlobalNamespace ?
                namespaceSymbol.FullQualifiedNameOmitGlobal() :
                string.Empty;
            bool hasNamespace = classNs.Length != 0;

            bool isInnerClass = symbol.ContainingType != null;

            string uniqueHint = symbol.FullQualifiedNameOmitGlobal().SanitizeQualifiedNameForUniqueHint()
                                + "_ScriptSerialization.generated";

            var source = new StringBuilder();

            source.Append("using Godot;\n");
            source.Append("using Godot.NativeInterop;\n");
            source.Append("\n");

            if (hasNamespace)
            {
                source.Append("namespace ");
                source.Append(classNs);
                source.Append(" {\n\n");
            }

            if (isInnerClass)
            {
                var containingType = symbol.ContainingType;

                while (containingType != null)
                {
                    source.Append("partial ");
                    source.Append(containingType.GetDeclarationKeyword());
                    source.Append(" ");
                    source.Append(containingType.NameWithTypeParameters());
                    source.Append("\n{\n");

                    containingType = containingType.ContainingType;
                }
            }

            source.Append("partial class ");
            source.Append(symbol.NameWithTypeParameters());
            source.Append("\n{\n");

            var members = symbol.GetMembers();

            var propertySymbols = members
                .Where(s => !s.IsStatic && s.Kind == SymbolKind.Property)
                .Cast<IPropertySymbol>()
                .Where(s => !s.IsIndexer);

            var fieldSymbols = members
                .Where(s => !s.IsStatic && s.Kind == SymbolKind.Field && !s.IsImplicitlyDeclared)
                .Cast<IFieldSymbol>();

            var godotClassProperties = propertySymbols.WhereIsGodotCompatibleType(typeCache).ToArray();
            var godotClassFields = fieldSymbols.WhereIsGodotCompatibleType(typeCache).ToArray();

            var signalDelegateSymbols = members
                .Where(s => s.Kind == SymbolKind.NamedType)
                .Cast<INamedTypeSymbol>()
                .Where(namedTypeSymbol => namedTypeSymbol.TypeKind == TypeKind.Delegate)
                .Where(s => s.GetAttributes()
                    .Any(a => a.AttributeClass?.IsGodotSignalAttribute() ?? false));

            List<GodotSignalDelegateData> godotSignalDelegates = new();

            foreach (var signalDelegateSymbol in signalDelegateSymbols)
            {
                if (!signalDelegateSymbol.Name.EndsWith(ScriptSignalsGenerator.SignalDelegateSuffix))
                    continue;

                string signalName = signalDelegateSymbol.Name;
                signalName = signalName.Substring(0,
                    signalName.Length - ScriptSignalsGenerator.SignalDelegateSuffix.Length);

                var invokeMethodData = signalDelegateSymbol
                    .DelegateInvokeMethod?.HasGodotCompatibleSignature(typeCache);

                if (invokeMethodData == null)
                    continue;

                godotSignalDelegates.Add(new(signalName, signalDelegateSymbol, invokeMethodData.Value));
            }

            source.Append(
                "    protected override void SaveGodotObjectData(global::Godot.Bridge.GodotSerializationInfo info)\n    {\n");
            source.Append("        base.SaveGodotObjectData(info);\n");

            // Save properties

            foreach (var property in godotClassProperties)
            {
                string propertyName = property.PropertySymbol.Name;

                source.Append("        info.AddProperty(PropertyName.")
                    .Append(propertyName)
                    .Append(", ")
                    .AppendManagedToVariantExpr(string.Concat("this.", propertyName),
                        property.PropertySymbol.Type, property.Type)
                    .Append(");\n");
            }

            // Save fields

            foreach (var field in godotClassFields)
            {
                string fieldName = field.FieldSymbol.Name;

                source.Append("        info.AddProperty(PropertyName.")
                    .Append(fieldName)
                    .Append(", ")
                    .AppendManagedToVariantExpr(string.Concat("this.", fieldName),
                        field.FieldSymbol.Type, field.Type)
                    .Append(");\n");
            }

            // Save signal events

            foreach (var signalDelegate in godotSignalDelegates)
            {
                string signalName = signalDelegate.Name;

                source.Append("        info.AddSignalEventDelegate(SignalName.")
                    .Append(signalName)
                    .Append(", this.backing_")
                    .Append(signalName)
                    .Append(");\n");
            }

            source.Append("    }\n");

            source.Append(
                "    protected override void RestoreGodotObjectData(global::Godot.Bridge.GodotSerializationInfo info)\n    {\n");
            source.Append("        base.RestoreGodotObjectData(info);\n");

            // Restore properties

            foreach (var property in godotClassProperties)
            {
                string propertyName = property.PropertySymbol.Name;

                source.Append("        if (info.TryGetProperty(PropertyName.")
                    .Append(propertyName)
                    .Append(", out var _value_")
                    .Append(propertyName)
                    .Append("))\n")
                    .Append("            this.")
                    .Append(propertyName)
                    .Append(" = ")
                    .AppendVariantToManagedExpr(string.Concat("_value_", propertyName),
                        property.PropertySymbol.Type, property.Type)
                    .Append(";\n");
            }

            // Restore fields

            foreach (var field in godotClassFields)
            {
                string fieldName = field.FieldSymbol.Name;

                source.Append("        if (info.TryGetProperty(PropertyName.")
                    .Append(fieldName)
                    .Append(", out var _value_")
                    .Append(fieldName)
                    .Append("))\n")
                    .Append("            this.")
                    .Append(fieldName)
                    .Append(" = ")
                    .AppendVariantToManagedExpr(string.Concat("_value_", fieldName),
                        field.FieldSymbol.Type, field.Type)
                    .Append(";\n");
            }

            // Restore signal events

            foreach (var signalDelegate in godotSignalDelegates)
            {
                string signalName = signalDelegate.Name;
                string signalDelegateQualifiedName = signalDelegate.DelegateSymbol.FullQualifiedNameIncludeGlobal();

                source.Append("        if (info.TryGetSignalEventDelegate<")
                    .Append(signalDelegateQualifiedName)
                    .Append(">(SignalName.")
                    .Append(signalName)
                    .Append(", out var _value_")
                    .Append(signalName)
                    .Append("))\n")
                    .Append("            this.backing_")
                    .Append(signalName)
                    .Append(" = _value_")
                    .Append(signalName)
                    .Append(";\n");
            }

            source.Append("    }\n");

            source.Append("}\n"); // partial class

            if (isInnerClass)
            {
                var containingType = symbol.ContainingType;

                while (containingType != null)
                {
                    source.Append("}\n"); // outer class

                    containingType = containingType.ContainingType;
                }
            }

            if (hasNamespace)
            {
                source.Append("\n}\n");
            }

            context.AddSource(uniqueHint, SourceText.From(source.ToString(), Encoding.UTF8));
        }
    }
}
