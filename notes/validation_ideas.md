*** Validation ideas

- confirm multimeasure rests span measures that exist
- confirm measure counts in each part (optionally: it may be permissible to have different numbers of measures per part. need to check)
- confirm layouts in "scores" have been defined in "layouts". This is required by the MNX doc and not enforced by the schema.
- confirm measures exist in staff systems
- confirm "part" and "labelref" values in staff and staff source objects contain valid values/part references (part ids)

*** Questions and Suggestions

- more symbol styles for groups (in particular, deskBracket)
- since the staff object has a "labelref" property, should it not also have a "part" property?
- staff group objects need a labelref alternative (and "part" that it references).
- there needs to be a mechanism to specify automatic name/shortName for "labelref" values.

